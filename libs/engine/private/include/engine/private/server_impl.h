#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <cstdint>

#include <engine/private/file_read_cache.h>
#include <engine/files_list.h>

namespace llbridge
{
class server_impl
{
public:
    struct file_loading
    {
        uint64_t session_id = -1;
        file_cache api;
    };

    server_impl(std::filesystem::path r, std::unique_ptr<server_transport_context> impl)
        : m_root(std::move(r))
        , m_transport(std::move(impl))

    {
    }

    server_transport_context&
    transport_context()
    {
        return *m_transport;
    }

    std::filesystem::path m_root;
    std::unordered_map<uint64_t, file_loading> m_file_sessions;

    file_list m_file_list;

    std::unique_ptr<server_transport_context> m_transport;
};

}  // namespace llbridge