#include <gtest/gtest.h>

#include "engine/private/utils.h"
#include "engine/private/client_transport_context.h"
#include "engine/private/server_context.h"
#include "engine/message.h"
#include "engine/communication_context.h"

using namespace llbridge;

TEST(utils, DISABLED_happy_path)
{
    llbridge::utils::file_competition fc;

    fc.init(950, 1000);

    ASSERT_EQ(fc.m_chunk_size, 1000);
    ASSERT_EQ(fc.m_file_size, 950);
    ASSERT_EQ(fc.m_mapping.size(), 1);

    fc.init(1050, 1000);

    ASSERT_EQ(fc.m_chunk_size, 1000);
    ASSERT_EQ(fc.m_file_size, 1050);
    ASSERT_EQ(fc.m_mapping.size(), 2);

    utils::rt_event er;

    std::thread t(
        [&er]()
        {
            std::this_thread::sleep_for(std::chrono::seconds{5});
            er.signal();
        });

    er.wait();
    t.join();
}

TEST(utils, test_messages)
{
    auto cctx = client_transport_context::make("127.0.0.1", "4012");
    communication_context client(&cctx->socket);

    auto sctx = server_transport_context::make("4012");
    sctx->start();

    utils::rt_event rt;

    std::unique_ptr<boost::asio::ip::tcp::socket> server_socket;

    sctx->m_acceptor.async_accept(
        [&rt, &server_socket](boost::system::error_code ec, boost::asio::ip::tcp::socket s)
        {
            server_socket = std::make_unique<boost::asio::ip::tcp::socket>(std::move(s));
            rt.signal();
        });

    boost::system::error_code ec;
    boost::asio::connect(cctx->socket, cctx->resolver.resolve(cctx->ip, cctx->port), ec);

    rt.wait();

    communication_context server(server_socket.get());

    file_list fl{.list = {{"file", 5}}};

    //     llbridge::list_message msg_to_send(fl);
    //
    //     server.send(std::move(msg_to_send));
    //
    //     llbridge::list_message result{};
    //     client.receive(result);
    //
    //     ASSERT_EQ(fl, result.dyn->fl);

    sctx->stop();
}
