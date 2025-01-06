#pragma once

#include <engine/files_list.h>
#include <engine/client.h>

#include <engine/private/file_read_cache.h>
#include <engine/private/secure_session.h>

#include <filesystem>
#include <memory>
#include <cstdint>

namespace llbridge
{
class client_transport_context;
class file_recombinator;

class client_impl
{
public:
    struct worker_ctx
    {
        uint64_t session_id = 0;
        uint64_t number_of_chunks = 0;
        uint64_t thread_id = 0;
        uint64_t offset = 0;
    };

    client_impl(std::unique_ptr<client_transport_context> impl,
                std::unique_ptr<secure_session_factory> secure,
                client::config cfg);

    client_transport_context&
    transport()
    {
        return *m_transport;
    }

    static void
    worker_thread(worker_ctx wctx,
                  client_transport_context& ctx,
                  std::unique_ptr<secure_session> secure,
                  file_recombinator& fapi);

    file_list m_lm;
    client::config m_cfg;
    std::unique_ptr<client_transport_context> m_transport;
    std::unique_ptr<secure_session_factory> m_secure_factory;
};

}  // namespace llbridge