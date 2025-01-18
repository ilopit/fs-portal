#include "engine/private/server_context.h"

namespace llbridge
{

std::unique_ptr<server_transport_context>
server_transport_context::make(std::string port)
{
    uint16_t parsed_port = std::stoi(port);
    return std::make_unique<server_transport_context>(parsed_port);
}

void
server_transport_context::start()
{
    for (int i = 0; i < 8; ++i)
    {
        m_workers.emplace_back(std::thread([this]() { m_io_context.run(); }));
    }
}

void
server_transport_context::stop()
{
    m_io_context.stop();

    for (auto& w : m_workers)
    {
        if (w.joinable())
        {
            w.join();
        }
    }
}

server_transport_context::server_transport_context(std::uint16_t port)
    : m_io_context()
    , m_work(m_io_context)
    , m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

}  // namespace llbridge