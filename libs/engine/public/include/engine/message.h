#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

#include <engine/files_list.h>
#include <spdlog/spdlog.h>

namespace llbridge
{

enum class message_type : uint8_t
{
    none = 101,
    hello,
    exit,
    list,
    download_file_session_request,
    download_file_session_response,
    download_file_session_end,
    chunk_response,
    chunk_request,
    response,
    error
};

template <typename fixed_part_type, typename extra_part_type, message_type mt>
struct message_impl
{
    constexpr static message_type
    get_message_type()
    {
        return mt;
    }

    constexpr static bool
    has_extra()
    {
        return !std::is_same_v<extra_part_type, char>;
    }

    template <typename... Args>
    message_impl&
    make_fixed(Args&&... args)
    {
        fixed = fixed_part_type(std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    message_impl&
    make_extra(Args&&... args)
    {
        static_assert(has_extra(), "should not be called on fixed only messages!");
        extra = extra_part_type(std::forward<Args>(args)...);
        return *this;
    }

    fixed_part_type fixed;
    extra_part_type extra;
};

template <typename fixed_part_type, message_type mt>
using fixed_message = message_impl<fixed_part_type, char, mt>;

template <typename fixed_part_type, typename extra_part_type, message_type mt>
using dynamic_message = message_impl<fixed_part_type, extra_part_type, mt>;

struct constants
{
    static constexpr uint64_t
    block_size()
    {
        return 16;
    }

    constexpr static uint64_t
    size_bytes(uint64_t size_in_blocks)
    {
        return size_in_blocks * block_size();
    }

    constexpr static uint64_t
    header_size()
    {
        return size_bytes(3);
    }

    constexpr static uint64_t
    fixed_content_size()
    {
        return size_bytes(8);
    }
};

struct message_header
{
    message_header()
        : iv()
        , aead()
        , extra_blocks(0)
        , extra(0)
        , version(1'000'000'000)
        , type(message_type::none)
    {
        iv.fill(0u);
        aead.fill(0u);
    }

    void
    set_extra_size(uint64_t size)
    {
        extra = (uint32_t)(size % constants::block_size());
        extra_blocks = (uint32_t)(size / constants::block_size() + ((uint64_t)extra ? 1 : 0));
    }

    uint64_t
    get_extra_size() const
    {
        return constants::size_bytes(extra_blocks - (extra ? 1 : 0)) + extra;
    }

    uint64_t
    encrypted_size() const
    {
        return encypted_fixed_size() + encypted_extra_size();
    }

    uint64_t
    encypted_extra_size() const
    {
        return extra_blocks ? (constants::size_bytes(extra_blocks) + constants::block_size()) : 0;
    }

    static constexpr uint64_t
    encypted_fixed_size()
    {
        return constants::fixed_content_size() + constants::header_size();
    }

    uint8_t*
    aead_begin()
    {
        return aead.data();
    }

    static uint64_t
    aead_size()
    {
        return offsetof(message_header, iv);
    }

    void
    print(spdlog::level::level_enum print_level);

    std::array<std::uint8_t, constants::block_size()> aead;
    alignas(4) uint32_t extra_blocks;
    alignas(4) uint32_t extra;
    alignas(4) uint32_t version;
    message_type type;
    std::array<std::uint8_t, constants::block_size()> iv;
};

static_assert(sizeof(message_header) == constants::header_size(), "Wrong header size!");

struct error_message_fixed_part
{
    error_message_fixed_part() = default;

    error_message_fixed_part(uint64_t e)
        : ec(e)
    {
    }

    uint64_t ec;
};

struct error_message_extra_part
{
    error_message_extra_part() = default;

    error_message_extra_part(std::string d)
        : details(std::move(d))
    {
    }

    uint64_t
    extract(std::vector<std::uint8_t>& v, uint64_t exra_size);

    uint64_t
    pack(std::vector<std::uint8_t>& v);

    std::string details;
};

using error_message =
    dynamic_message<error_message_fixed_part, error_message_extra_part, message_type::error>;

struct download_file_session_request_fixed_part
{
    download_file_session_request_fixed_part() = default;

    download_file_session_request_fixed_part(uint64_t i, uint64_t size, uint64_t o)
        : id(i)
        , chunk_size(size)
        , offset(o)
    {
    }

    uint64_t id;
    uint64_t chunk_size;
    uint64_t offset;
};

using download_file_session_request = fixed_message<download_file_session_request_fixed_part,
                                                    message_type::download_file_session_request>;

struct download_file_session_response_fixed_part
{
    download_file_session_response_fixed_part() = default;

    download_file_session_response_fixed_part(uint64_t id, uint64_t chunk_size)
        : session_id(id)
        , number_of_chunks(chunk_size)
    {
    }

    uint64_t session_id;
    uint64_t number_of_chunks;
};

using download_file_session_response = fixed_message<download_file_session_response_fixed_part,
                                                     message_type::download_file_session_response>;

struct download_file_session_end_fixed_part
{
    download_file_session_end_fixed_part() = default;

    download_file_session_end_fixed_part(uint64_t id)
        : session_id(id)
    {
    }

    uint64_t session_id;
};

using download_file_session_end =
    fixed_message<download_file_session_end_fixed_part, message_type::download_file_session_end>;

struct chunk_request_fixed_part
{
    chunk_request_fixed_part() = default;

    chunk_request_fixed_part(uint64_t sid, uint64_t cid)
        : session_id(sid)
        , chunk_id(cid)
    {
    }

    uint64_t session_id;
    uint64_t chunk_id;
};

using chunk_request = fixed_message<chunk_request_fixed_part, message_type::chunk_request>;

struct chunk_response_fixed_part
{
    chunk_response_fixed_part() = default;

    chunk_response_fixed_part(uint64_t sid, uint64_t cid)
        : session_id(sid)
        , chunk_id(cid)
    {
    }

    uint64_t session_id;
    uint64_t chunk_id;
};

struct chunk_response_extra_part
{
    chunk_response_extra_part() = default;

    chunk_response_extra_part(std::vector<std::uint8_t> d)
        : data(std::move(d))
    {
    }

    uint64_t
    extract(std::vector<std::uint8_t>& v, uint64_t exra_siz);

    uint64_t
    pack(std::vector<std::uint8_t>& v);

    std::vector<std::uint8_t> data;
};

using chunk_response = dynamic_message<chunk_request_fixed_part,
                                       chunk_response_extra_part,
                                       message_type::chunk_response>;

struct hello_request_fixed_part
{
};

using hello_request = fixed_message<hello_request_fixed_part, message_type::hello>;

struct list_message_fixed_part
{
};

struct list_message_extra_part
{
    list_message_extra_part() = default;

    list_message_extra_part(file_list v)
        : files(std::move(v))
    {
    }

    uint64_t
    extract(std::vector<std::uint8_t>& extra_content, uint64_t exra_siz);

    uint64_t
    pack(std::vector<std::uint8_t>& extra_content);

    file_list files;
};

using list_message =
    dynamic_message<list_message_fixed_part, list_message_extra_part, message_type::list>;

}  // namespace llbridge
