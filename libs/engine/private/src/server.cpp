#include "engine/server.h"

#include "engine/private/session.h"
#include "engine/private/server_context.h"
#include "engine/private/server_impl.h"

#include <boost/asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

namespace llbridge
{

server::server(std::shared_ptr<server_impl> impl)
    : m_impl(std::move(impl))
{
}

server::server() = default;

server::~server()
{
    stop();
}

server
server::make(const config& cfg)
{
    auto ctx = server_transport_context::make(cfg.port);

    auto impl = std::make_unique<server_impl>(cfg.root_dir, std::move(ctx));

    impl->m_file_list = file_list::init(cfg.root_dir);

    return server(std::move(impl));
}

bool
server::start()
{
    m_impl->transport_context().start();

    start_impl();

    return true;
}

void
server::start_impl()
{
    // this is an async accept which means the lambda function is
    // executed, when a client connects
    m_impl->transport_context().m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                SPDLOG_INFO("Creating session on {0}:{1}",
                            socket.remote_endpoint().address().to_string(),
                            socket.remote_endpoint().port());

                // create a session where we immediately call the run function
                // note: the socket is passed to the lambda here
                std::make_shared<connection_session>(m_impl, std::move(socket))->run();
            }
            else
            {
                SPDLOG_ERROR("Creating session failed {0}", ec.message());
            }
            // since we want multiple clients to connect, wait for the next one by calling
            // do_accept()
            start_impl();
        });
}

bool
server::stop()
{
    m_impl->transport_context().stop();

    return true;
}

}  // namespace llbridge