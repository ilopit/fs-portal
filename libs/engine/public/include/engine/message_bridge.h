#pragma once

#include <engine/message.h>

#include <spdlog/spdlog.h>
#include "botan_all.h"

namespace llbridge
{
class secure_session;

struct message_bridge
{
    message_bridge(std::unique_ptr<secure_session> secure);
    message_bridge();
    ~message_bridge();

    message_header*
    get_header()
    {
        return (message_header*)fctx.data();
    }

    template <typename T>
    bool
    write(T t)
    {
        bool result = get_header()->type == message_type::none;
        if (!result)
        {
            SPDLOG_ERROR("Incompatible types {} != {}", (int)T::get_message_type(),
                         (int)get_header()->type);

            return result;
        }

        get_header()->type = T::get_message_type();

        fixed_write(t.fixed);

        if constexpr (T::has_extra())
        {
            auto dyn_size = t.extra.pack(dctx);
            get_header()->set_extra_size(dyn_size);
        }

        result = encrypt();
        get_header()->print(spdlog::level::debug);

        return result;
    }

    template <typename T>
    bool
    read(T& t)
    {
        bool result = T::get_message_type() == get_header()->type;
        if (!result)
        {
            SPDLOG_ERROR("Incompatible types {} != {}", (int)T::get_message_type(),
                         (int)get_header()->type);
            return result;
        }

        result = decrypt();
        if (!result)
        {
            return result;
        }

        fixed_read(t.fixed);

        if constexpr (T::has_extra())
        {
            t.extra.extract(dctx, get_header()->get_extra_size());
        }

        return result;
    }

    std::vector<std::uint8_t>&
    fixed_ctx()
    {
        return fctx;
    }

    std::vector<std::uint8_t>&
    dynamic_ctx()
    {
        return dctx;
    }

    void
    reset()
    {
        *get_header() = message_header{};
    }

private:
    template <typename T>
    void
    fixed_write(const T& t)
    {
        std::memcpy(fctx.data() + constants::header_size(), &t, sizeof(T));
    }

    template <typename T>
    void
    fixed_read(T& t)
    {
        std::memcpy(&t, fctx.data() + constants::header_size(), sizeof(T));
    }

    bool
    encrypt() noexcept;

    bool
    decrypt() noexcept;

    std::unique_ptr<secure_session> m_encyption_layer;

    std::vector<std::uint8_t> fctx;
    std::vector<std::uint8_t> dctx;
};
}  // namespace llbridge