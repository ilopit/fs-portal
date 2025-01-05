#include <engine/message_bridge.h>

#include <engine/private/secure_session.h>
#include <spdlog/spdlog.h>

namespace llbridge
{

message_bridge::message_bridge(std::unique_ptr<secure_session> secure)
    : dctx(1024 * 1024, 0u)
    , m_encyption_layer(std::move(secure))
{
    fctx.resize(message_header::encypted_fixed_size(), 0u);

    get_header()->type = message_type::none;
}
message_bridge::message_bridge()
{
}
message_bridge::~message_bridge()
{
}

bool
message_bridge::encrypt()
{
    try
    {
        m_encyption_layer->encrypt(*get_header(), fctx, dctx);
    }
    catch (std::exception& e)
    {
        SPDLOG_ERROR("Encrypt failed {}", e.what());
        return false;
    }

    return true;
}

bool
message_bridge::decrypt()
{
    try
    {
        m_encyption_layer->decrypt(*get_header(), fctx, dctx);
    }
    catch (std::exception& e)
    {
        SPDLOG_ERROR("Decrypt failed {}", e.what());
        return false;
    }

    return true;
}

}  // namespace llbridge