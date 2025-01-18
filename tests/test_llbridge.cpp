#include <gtest/gtest.h>

#include <engine/server.h>
#include <engine/client.h>
#include <engine/communication_context.h>
#include <filesystem>

#include "testing_utils.h"

namespace
{
const std::filesystem::path big_file = "super_secret_file";
const std::filesystem::path server_root = "server_root";
const std::filesystem::path client_root = "client_root";
}  // namespace

struct test_llbridge : public ::testing::Test
{
    void
    SetUp()
    {
        spdlog::set_level(spdlog::level::warn);

        testing_utils::make_a_secret();

        std::error_code ec;
        std::filesystem::remove(server_root, ec);
        std::filesystem::create_directory(server_root, ec);

        auto str = fmt::format("openssl rand -out {}/{} --base64 {}", server_root.generic_string(),
                               big_file.generic_string(), (1 << 30));
        ::system(str.c_str());
    }
};

TEST_F(test_llbridge, sync_file)
{
    auto srv = llbridge::server::make({.port = "5051", .root = server_root, .secret = "secret"});
    ASSERT_TRUE(srv.start());

    auto client = llbridge::client::make({.port = "5051",
                                          .ip = "127.0.0.1",
                                          .root = std::filesystem::absolute(client_root),
                                          .secret = "secret"});

    ASSERT_TRUE(client.open());

    ASSERT_TRUE(client.sync(0));

    srv.stop();

    auto str =
        fmt::format("cmp {}/{} {}/{}", server_root.generic_string(), big_file.generic_string(),
                    client_root.generic_string(), big_file.generic_string());
    ASSERT_EQ(::system(str.c_str()), 0);
}
