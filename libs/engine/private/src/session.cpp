#include "engine/private/session.h"

#include <iostream>

#include <engine/server.h>
#include <engine/message.h>
#include <engine/private/server_context.h>
#include <engine/private/server_impl.h>
#include <engine/private/stats.h>

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

namespace llbridge
{
namespace
{

std::vector<std::string>
get_available_files()
{
    auto root = std::filesystem::current_path();

    std::vector<std::string> result;

    for (auto& p : std::filesystem::recursive_directory_iterator(root))
    {
        if (!p.is_directory())
        {
            auto file = std::filesystem::relative(p, root);

            result.push_back(file.string());
        }
    }

    return result;
}

}  // namespace

connection_session::connection_session(std::shared_ptr<server_impl> ss,
                                       boost::asio::ip::tcp::socket socket,
                                       statistics* s)
    : m_server_impl(std::move(ss))
    , m_socket(std::move(socket))
    , m_ctx(&m_socket, ss->m_secure_session->create_session(), s)
{
}

void
connection_session::run()
{
    wait_for_message(message_header::encypted_fixed_size());
}

connection_session::message_result
connection_session::on_hello(const hello_request&)
{
    auto lm = list_message().make_extra(m_server_impl->m_file_list);

    if (!m_ctx.send(std::move(lm)))
    {
        return message_result::error;
    }
    return message_result::next;
}

connection_session::message_result
connection_session::on_download_file_request(const download_file_session_request& dfs_request)
{
    if (dfs_request.fixed.id >= m_server_impl->m_file_list.list.size())
    {
        SPDLOG_ERROR("File if [{0}] doesn't exist ", dfs_request.fixed.id);
        return message_result::error;
    }

    auto path = m_server_impl->m_root;
    path /= m_server_impl->m_file_list.list[dfs_request.fixed.id].path;

    static uint64_t session_id = 0;
    auto current_sid = ++session_id;

    auto& f = m_server_impl->m_file_sessions[session_id];

    file_cache::open_config oc{.file = path,
                               .chunk_size = dfs_request.fixed.chunk_size,
                               .cache_size = 16,
                               .read_in_advance = 16};
    if (!f.api.start(oc))
    {
        return message_result::error;
    }

    auto dfs_response =
        download_file_session_response().make_fixed(current_sid, f.api.number_of_chunks());
    if (!m_ctx.send(std::move(dfs_response)))
    {
        return message_result::error;
    }

    return message_result::next;
}

connection_session::message_result
connection_session::on_download_session_end(const download_file_session_end& df)
{
    auto itr = m_server_impl->m_file_sessions.find(df.fixed.session_id);

    if (itr == m_server_impl->m_file_sessions.end())
    {
        SPDLOG_ERROR("Session [{0}] doesn't exist ", df.fixed.session_id);
        return message_result::error;
    }

    return message_result::next;
}

connection_session::message_result
connection_session::on_chunk_request(const chunk_request& c_request)
{
    auto itr = m_server_impl->m_file_sessions.find(c_request.fixed.session_id);

    if (itr == m_server_impl->m_file_sessions.end())
    {
        SPDLOG_ERROR("Session [{0}] doesn't exist ", c_request.fixed.session_id);
        return message_result::error;
    }

    auto& fapi = itr->second.api;

    auto rqst = fapi.read_chunk(c_request.fixed.chunk_id);

    data_chunk_ptr dcp = nullptr;
    if (!rqst->read_data(dcp))
    {
        return message_result::error;
    }

    auto old_buf = cc().extract_dctx();

    auto c_response =
        chunk_response().make_fixed(c_request.fixed.session_id, c_request.fixed.chunk_id);

    std::swap(c_response.extra.data, dcp->data);
    std::swap(old_buf, dcp->data);

    cc().send(std::move(c_response));

    fapi.release_free_chunk(std::move(dcp));

    return message_result::next;
}

void
connection_session::on_message_received()
{
    message_result result = message_result::end;
    auto type = cc().m_bridge.get_header()->type;
    switch (type)
    {
    case message_type::hello:
    {
        hello_request hr{};
        if (m_ctx.read(hr))
        {
            result = on_hello(hr);
        }
        else
        {
            result = message_result::error;
        }
        break;
    }
    case message_type::download_file_session_request:
    {
        download_file_session_request dfs_request{};
        m_ctx.read(dfs_request);
        result = on_download_file_request(dfs_request);

        break;
    }
    case message_type::chunk_request:
    {
        chunk_request cr{};
        m_ctx.read(cr);
        result = on_chunk_request(cr);

        break;
    }
        //     case message_type::download_file_end:
        //     {
        //         //         download_file_session_end dse;
        //         //         dse.mh = m_current_header;
        //         //         dse.load(ctx);
        //         //         result = on_donwload_session_end(dse);
        //
        //         break;
        //     }
    default:
        SPDLOG_ERROR("Unsupported message", (int)result);
        result = message_result::error;
    }

    if (result == message_result::error || result == message_result::end)
    {
        SPDLOG_ERROR("Error {} {} ", (int)type, (int)result);
        error_message em{};

        m_ctx.m_bridge.reset();
        if (!m_ctx.send(std::move(em)))
        {
            SPDLOG_ERROR("Sending error failed");
        }

        m_ctx.m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        m_ctx.m_socket->close();
    }
}

auto s_last_time = std::chrono::high_resolution_clock::now();

void
connection_session::wait_for_message(uint64_t expected_size)
{
    // since we capture `this` in the callback, we need to call shared_from_this()
    auto self(shared_from_this());
    // and now call the lambda once data arrives
    // we read a string until the null termination character

    auto msg = [self](bool valid)
    {
        if (valid)
        {
            self->on_message_received();
            self->wait_for_message(0);
        }
    };

    m_ctx.wait_for_msg(msg);
}
}  // namespace llbridge