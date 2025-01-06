#include <engine/private/client_transport_context.h>

#include <spdlog/spdlog.h>

namespace llbridge
{

client_transport_context::client_transport_context()
    : socket(io_context)
    , resolver(io_context)
{
}

client_transport_context::client_transport_context(std::string i, std::string p)
    : socket(io_context)
    , resolver(io_context)
    , ip(std::move(i))
    , port(std::move(p))
{
}

std::unique_ptr<connection>
client_transport_context::connect()
{
    auto result = std::make_unique<connection>(boost::asio::ip::tcp::socket{io_context});
    boost::system::error_code ec;
    boost::asio::connect(result->m_socket, resolver.resolve(ip, port), ec);

    if (ec)
    {
        SPDLOG_ERROR("Connection failed {}", ec.what());
        return nullptr;
    }

    return result;
}

std::unique_ptr<client_transport_context>
client_transport_context::make(std::string ip, std::string port)
{
    auto ctx = std::make_unique<client_transport_context>(std::move(ip), std::move(port));

    return ctx;
}

}  // namespace llbridge