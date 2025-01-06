#include <gtest/gtest.h>

#include <engine/server.h>
#include <engine/client.h>
#include <engine/communication_context.h>

#include "testing_utils.h"

TEST(test_llbridge, sync_file)
{
    testing_utils::make_a_secret();

    std::error_code ec;
    std::filesystem::remove("sd//rr", ec);

    auto srv = llbridge::server::make({.port = "5051", .root = "D://movies", .secret = "secret"});
    ASSERT_TRUE(srv.start());

    auto client = llbridge::client::make({.port = "5051", .ip = "127.0.0.1", .secret = "secret"});

    ASSERT_TRUE(client.open());

    ASSERT_TRUE(client.sync(3));

    srv.stop();

    auto result = system("cmp D://movies//sd//rr root//sd//rr");
    ASSERT_EQ(result, 0);
}
