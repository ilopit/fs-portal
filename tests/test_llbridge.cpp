#include <gtest/gtest.h>

#include <engine/server.h>
#include <engine/client.h>
#include <engine/communication_context.h>

TEST(test_llbridge, sync_file)
{
    std::error_code ec;
    std::filesystem::remove("sd//rr", ec);

    auto srv = llbridge::server::make({.port = "5051", .root_dir = "D://movies"});
    ASSERT_TRUE(srv.start());

    auto client = llbridge::client::make({.port = "5051", .ip = "127.0.0.1"});

    ASSERT_TRUE(client.open());

    ASSERT_TRUE(client.sync(3));

    // std::this_thread::sleep_for(std::chrono::seconds(40));
    srv.stop();

    auto result = system("cmp D://movies//sd//rr sd//rr");
    ASSERT_EQ(result, 0);
}
