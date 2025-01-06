#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <cstdint>

#include <engine/files_list.h>
#include <engine/private/file_read_cache.h>
#include <engine/private/secure_session.h>
#include <engine/private/stats.h>

namespace llbridge
{

class secure_session_factory;

class server_impl
{
public:
    struct file_loading
    {
        uint64_t session_id = -1;
        file_cache api;
    };

    server_impl(std::filesystem::path r,
                std::unique_ptr<server_transport_context> transport,
                std::unique_ptr<secure_session_factory> ssf)
        : m_root(std::move(r))
        , m_file_sessions()
        , m_file_list()
        , m_transport(std::move(transport))
        , m_secure_session(std::move(ssf))
        , m_statistics(std::chrono::milliseconds(2000))

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
    std::unique_ptr<secure_session_factory> m_secure_session;
    statistics m_statistics;
};

}  // namespace llbridge