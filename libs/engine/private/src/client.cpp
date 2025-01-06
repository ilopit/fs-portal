#include <engine/client.h>

#include <boost/asio.hpp>
#include <filesystem>

#include "engine/communication_context.h"

#include "engine/message.h"
#include "engine/private/client_impl.h"
#include "engine/private/file_recombinator.h"
#include "engine/private/secure_session.h"
#include "engine/private/client_transport_context.h"
#include "engine/private/utils.h"

#include <spdlog/spdlog.h>
#include <iostream>

namespace llbridge
{

client
client::make(config cfg)
{
    cfg.print();

    auto ctx = client_transport_context::make(cfg.ip, cfg.port);
    if (!ctx)
    {
        return {};
    }
    auto secure = secure_session_factory::create(cfg.secret);
    if (!secure)
    {
        return {};
    }

    auto impl = std::make_unique<client_impl>(std::move(ctx), std::move(secure), std::move(cfg));
    if (!impl)
    {
        return {};
    }

    return client(std::move(impl));
}

client::~client(){};

bool
client::open()
{
    auto conn = m_impl->m_transport->connect();
    if (!conn)
    {
        SPDLOG_ERROR("Failed to open");
        return false;
    }

    hello_request hr{};

    communication_context cc(&conn->socket(), m_impl->m_secure_factory->create_session());
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

void
client::print_list()
{
    m_impl->m_lm.print();
}

client::client(std::unique_ptr<client_impl> impl)
    : m_impl(std::move(impl))
{
}

client::client()
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
    auto conn = m_impl->transport().connect();

    if (!conn)
    {
        return false;
    }

    std::filesystem::path p = m_impl->m_cfg.root / m_impl->m_lm.list[file_id].path;

    file_recombinator fapi;

    file_recombinator::open_config wc{
        .file = p,
        .chunk_size = m_impl->m_cfg.block_size,
        .file_size = m_impl->m_lm.list[file_id].size,
        .batch_size = m_impl->m_cfg.blocks_in_batch,
    };

    auto r = fapi.start(wc);

    if (!r)
    {
        return false;
    }

    auto resule_cunks = fapi.get_completed_chunks();

    communication_context cc(&conn->socket(), m_impl->m_secure_factory->create_session());

    auto df_request = download_file_session_request().make_fixed(file_id, m_impl->m_cfg.block_size,
                                                                 fapi.get_resume_point());

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

    client_impl::worker_ctx thread_context{.session_id = dfs_response.fixed.session_id,
                                           .number_of_chunks = dfs_response.fixed.number_of_chunks,
                                           .offset = m_impl->m_cfg.number_of_loaders};

    std::vector<std::thread> threads;

    for (int i = 0; i < m_impl->m_cfg.number_of_loaders; ++i)
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

bool
client::config::update_from_config(const std::filesystem::path& p)
{
    auto loaders = utils::load_commmnds_builder()
                       .add("port", port)
                       .add("ip", ip)
                       .add("root", root)
                       .add("secret", secret)
                       .add("block_size", block_size)
                       .add("number_of_loaders", number_of_loaders)
                       .add("blocks_in_batch", blocks_in_batch)
                       .finalize();

    return utils::generic_kv::load(p, loaders);
}

void
client::config::print()
{
    constexpr auto r =
        R"(******* client::config *******
             port: {}
               ip: {}
             root: {}
           secret: {}
       block_size: {}
number_of_loaders: {}
  blocks_in_batch: {}
)";

    std::cout << fmt::format(r, port, ip, root.generic_string(), secret.generic_string(),
                             block_size, number_of_loaders, blocks_in_batch)
              << std::endl;
}

}  // namespace llbridge