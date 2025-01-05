#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <engine/message.h>
#include <engine/message_bridge.h>
#include <spdlog/spdlog.h>

// Primary template (default case: false)
template <template <typename...> class Template, typename T>
struct belongs_to_template : std::false_type
{
};

// Partial specialization (true if T matches the Template instance)
template <template <typename...> class Template, typename... Args>
struct belongs_to_template<Template, Template<Args...>> : std::true_type
{
};

// Helper variable template for convenience
template <template <typename...> class Template, typename T>
constexpr bool belongs_to_template_v = belongs_to_template<Template, T>::value;

namespace llbridge
{
struct message_header;
class connection_session;

struct communication_context
{
    communication_context() = default;

    communication_context(boost::asio::ip::tcp::socket* s, std::unique_ptr<secure_session> secure);

    template <typename T>
    bool
    send(T c)
    {
        auto result = m_bridge.write(std::move(c));
        if (!result)
        {
            SPDLOG_INFO("bridge write failed");
            goto clean_up;
        }

        result = send_bytes(m_bridge.get_header()->encrypted_size());

        if (!result)
        {
            SPDLOG_INFO("send error");
        }
    clean_up:
        m_bridge.reset();

        return result;
    }

    void
    wait_for_msg(std::function<void(bool)> handler);

    template <typename T>
    bool
    receive(T& c)
    {
        auto result = receive_fixed_bytes();
        if (!result)
        {
            SPDLOG_ERROR("Size missmatch [0] != [1]", result, m_bridge.fixed_ctx().size());
            goto clean_up;
        }

        if (m_bridge.get_header()->encypted_extra_size() > 0)
        {
            auto expected = m_bridge.get_header()->encypted_extra_size();
            auto size = receive_extra_bytes(expected);
            result = expected == size;
            if (!result)
            {
                SPDLOG_ERROR("Size missmatch {} != {}", size, expected);
                goto clean_up;
            }
        }

        result = read(c);

    clean_up:
        m_bridge.reset();

        return result;
    }

    template <typename T>
    bool
    read(T& c)
    {
        auto result = m_bridge.read(c);

        m_bridge.reset();

        return result;
    }

    void
    receive_async(std::shared_ptr<connection_session> self);

    void
    on_init_received()
    {
    }

    boost::array<boost::asio::mutable_buffer, 2>
    make_buffers()
    {
        return {boost::asio::buffer(m_bridge.fixed_ctx()),
                boost::asio::buffer(m_bridge.dynamic_ctx())};
    }

    bool
    receive_fixed_bytes();

    uint64_t
    receive_extra_bytes(uint64_t expected);

    uint64_t
    send_bytes(uint64_t expected);

    std::vector<uint8_t>
    extract_dctx()
    {
        return std::move(m_bridge.dynamic_ctx());
    }

    void
    replace_dctx(std::vector<uint8_t>& dcx)
    {
        return std::swap(m_bridge.dynamic_ctx(), dcx);
    }

    message_bridge m_bridge;

    boost::asio::ip::tcp::socket* m_socket = nullptr;
};
}  // namespace llbridge