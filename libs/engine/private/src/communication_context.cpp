#include <engine/communication_context.h>

#include "engine/message.h"

#include <iostream>
#include <spdlog/spdlog.h>
#include <boost/array.hpp>

#include "engine/private/session.h"
#include "engine/private/secure_session.h"
#include "engine/private/stats.h"

using byte = unsigned char;

namespace llbridge
{

uint64_t
communication_context::receive_extra_bytes(uint64_t expected)
{
    if (!m_socket)
    {
        return 0;
    }

    if (m_bridge.dynamic_ctx().size() < expected)
    {
        if (m_bridge.dynamic_ctx().capacity() < expected)
        {
            SPDLOG_INFO("Performance hit! Resizing dctx with capacity {} => {}",
                        m_bridge.dynamic_ctx().capacity(), expected);
        }

        m_bridge.dynamic_ctx().resize(expected);
    }

    boost::system::error_code ec;

    auto size = boost::asio::read(*m_socket, boost::asio::buffer(m_bridge.dynamic_ctx()),
                                  boost::asio::transfer_exactly(expected), ec);

    if (ec || size != expected)
    {
        SPDLOG_ERROR("receive_bytes failed [0]", ec.what());
    }
    m_stats->update_received(expected);

    SPDLOG_TRACE("Received {}", size);

    return size;
}

bool
communication_context::receive_fixed_bytes()
{
    if (!m_socket)
    {
        return false;
    }
    boost::system::error_code ec;
    constexpr auto msg_size = message_header::encypted_fixed_size();

    auto size = boost::asio::read(*m_socket, boost::asio::buffer(m_bridge.fixed_ctx()), ec);

    auto error = ec || size != msg_size;
    if (error)
    {
        SPDLOG_ERROR("receive_bytes failed {}", ec.what());
    }
    m_stats->update_received(size);

    SPDLOG_TRACE("Received {}", size);

    return !error;
}

uint64_t
communication_context::send_bytes(uint64_t expected)
{
    if (!m_socket)
    {
        return 0;
    }

    boost::system::error_code ec;
    auto size =
        boost::asio::write(*m_socket, make_buffers(), boost::asio::transfer_exactly(expected), ec);

    if (ec || size != expected)
    {
        SPDLOG_ERROR("send_bytes failed [0]", ec.what());
    }
    m_stats->update_send(size);

    SPDLOG_TRACE("Sended {}", size);

    return size;
}

communication_context::communication_context(boost::asio::ip::tcp::socket* s,
                                             std::unique_ptr<secure_session> secure,
                                             statistics* stats)
    : m_socket(s)
    , m_bridge(std::move(secure))
    , m_stats(stats)
{
}

void
communication_context::wait_for_msg(std::function<void(bool)> handler)
{
    constexpr auto size = message_header::encypted_fixed_size();
    boost::asio::async_read(
        *m_socket, boost::asio::buffer(m_bridge.fixed_ctx(), size),
        boost::asio::transfer_exactly(size),
        [this, size, handler](const boost::system::error_code& error, std::size_t bytes_transferred)
        {
            m_stats->update_received(bytes_transferred);
            if (size != bytes_transferred || error)
            {
                handler(false);
                return;
            }
            auto extra_size = m_bridge.get_header()->encypted_extra_size();

            if (extra_size == 0)
            {
                handler(true);
            }
            else
            {
                boost::asio::async_read(
                    *m_socket, boost::asio::buffer(m_bridge.dynamic_ctx()),
                    boost::asio::transfer_exactly(extra_size),
                    [extra_size, handler, this](const boost::system::error_code& error,
                                                std::size_t bytes_transferred)
                    {
                        m_stats->update_received(bytes_transferred);
                        handler(extra_size == bytes_transferred && !error);
                        return true;
                    });
            }
        });
}

}  // namespace llbridge