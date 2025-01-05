#include <engine/private/client_transport_context.h>

namespace llbridge
{

client_transport_context::client_transport_context()
    : socket(io_context)
    , resolver(io_context)
{
}

std::unique_ptr<client_transport_context>
client_transport_context::make(std::string ip, std::string port)
{
    auto ctx = std::make_unique<client_transport_context>();

    ctx->ip = std::move(ip);
    ctx->port = std::move(port);

    return ctx;
}

}  // namespace llbridge