#include "engine/private/client_impl.h"

#include "engine/private/client_transport_context.h"
#include "engine/communication_context.h"
#include "engine/private/file_recombinator.h"

#include <boost/asio.hpp>

void
llbridge::client_impl::worker_thread(worker_ctx wctx,
                                     client_transport_context& ctx,
                                     std::unique_ptr<secure_session> secure,
                                     file_recombinator& fapi)
{
    boost::asio::ip::tcp::socket socket{ctx.io_context};
    boost::asio::connect(socket, ctx.resolver.resolve(ctx.ip, ctx.port));

    SPDLOG_WARN("Worker started {}", wctx.thread_id);

    communication_context cc(&socket, std::move(secure));

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

    boost::system::error_code ec;

    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close();
}
