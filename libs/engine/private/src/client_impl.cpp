#include "engine/private/client_impl.h"

#include "engine/communication_context.h"

#include "engine/private/client_transport_context.h"
#include "engine/private/file_recombinator.h"

#include <boost/asio.hpp>

namespace llbridge
{

client_impl::client_impl(std::unique_ptr<client_transport_context> impl,
                         std::unique_ptr<secure_session_factory> secure,
                         client::config cfg)
    : m_lm()
    , m_cfg(std::move(cfg))
    , m_transport(std::move(impl))
    , m_secure_factory(std::move(secure))
    , m_stats(std::chrono::milliseconds(2000))
{
}

void
client_impl::worker_thread(worker_ctx wctx,
                           client_transport_context& ctx,
                           std::unique_ptr<secure_session> secure,
                           file_recombinator& fapi,
                           statistics* s)
{
    auto conn = ctx.connect();
    if (!conn)
    {
        return;
    }

    SPDLOG_WARN("Worker started {}", wctx.thread_id);

    communication_context cc(&conn->socket(), std::move(secure), s);

    std::vector<write_result_ptr> requests;

    for (uint64_t cid = wctx.thread_id; cid < wctx.number_of_chunks; cid += wctx.offset)
    {
        auto chunk_reques = chunk_request().make_fixed(wctx.session_id, cid);

        auto result = cc.send(std::move(chunk_reques));
        if (!result)
        {
            SPDLOG_ERROR("send error {0}", cid);
            return;
        }

        {
            auto fc = fapi.allocate_free_chunk(0);

            cc.replace_dctx(fc->data);
        }

        chunk_response c_response;
        result = cc.receive(c_response);
        if (!result)
        {
            SPDLOG_ERROR("file_api error {0}", cid);
            return;
        }

        if (c_response.fixed.chunk_id != cid || c_response.fixed.session_id != wctx.session_id)
        {
            SPDLOG_ERROR("Chunk doesn't belong to file session");
            return;
        }

        auto w_result = fapi.write_chunk(std::make_unique<data_chunk>(
            c_response.fixed.chunk_id, std::move(c_response.extra.data)));

        requests.emplace_back(std::move(w_result));
    }

    for (auto& r : requests)
    {
        r->wait();
    }

    SPDLOG_WARN("Worker end {}", wctx.thread_id);
}

}  // namespace llbridge