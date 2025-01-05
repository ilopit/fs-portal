#include <engine/client.h>

#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "engine/communication_context.h"

#include "engine/message.h"
#include "engine/private/utils.h"
#include "engine/private/client_impl.h"
#include "engine/private/file_recombinator.h"
#include "engine/private/secure_session.h"
#include "engine/private/client_transport_context.h"

#include <spdlog/spdlog.h>

namespace llbridge
{

client
client::make(const config& cfg)
{
    auto ctx = client_transport_context::make(cfg.ip, cfg.port);
    auto secure = secure_session_factory::create(cfg.secret);

    auto impl = std::make_unique<client_impl>(std::move(ctx), std::move(secure), cfg.root);

    return client(std::move(impl));
}

client::client() = default;
client::~client() = default;

bool
client::open()
{
    boost::asio::connect(
        m_impl->transport().socket,
        m_impl->transport().resolver.resolve(m_impl->transport().ip, m_impl->transport().port));

    hello_request hr{};

    communication_context cc(&m_impl->transport().socket,
                             m_impl->m_secure_factory->create_session());
    bool result = cc.send(std::move(hr));
    if (!result)
    {
        return false;
    }

    list_message lm;
    result = cc.receive(lm);
    if (!result)
    {
        return false;
    }
    m_impl->m_lm = std::move(lm.extra.files);

    m_impl->m_lm.print();

    return true;
}

const file_list&
client::list()
{
    return m_impl->m_lm;
}

client::client(std::unique_ptr<client_impl> impl)
    : m_impl(std::move(impl))
{
}

bool
client::sync_all()
{
    return true;
}

bool
client::sync(uint64_t file_id)
{
    communication_context cc(&m_impl->transport().socket,
                             m_impl->m_secure_factory->create_session());
    const uint64_t chunk_size = 1024 * 1024 * 8;
    auto df_request = download_file_session_request().make_fixed(file_id, chunk_size);

    auto result = cc.send(std::move(df_request));
    if (!result)
    {
        return false;
    }

    download_file_session_response dfs_response{};
    result = cc.receive(dfs_response);
    if (!result)
    {
        return false;
    }

    std::filesystem::path p = m_impl->m_root / m_impl->m_lm.list[file_id].path;

    file_recombinator fapi;

    file_recombinator::open_config wc{.file = p,
                                      .chunk_size = chunk_size,
                                      .file_size = m_impl->m_lm.list[file_id].size,
                                      .batch_size = 8};

    auto r = fapi.start(wc);

    if (!r)
    {
        return false;
    }

    const auto number_of_workers = 8;

    client_impl::worker_ctx thread_context{.session_id = dfs_response.fixed.session_id,
                                           .number_of_chunks = dfs_response.fixed.number_of_chunks,
                                           .offset = number_of_workers};

    std::vector<std::thread> threads;

    for (int i = 0; i < number_of_workers; ++i)
    {
        thread_context.thread_id = i;
        threads.emplace_back(client_impl::worker_thread, thread_context,
                             std::ref(m_impl->transport()),
                             m_impl->m_secure_factory->create_session(), std::ref(fapi));
    }

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    fapi.stop();

    return true;
}

}  // namespace llbridge