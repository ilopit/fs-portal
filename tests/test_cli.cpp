#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>

#include "testing_utils.h"

int
server_main(int argc, char** argv);

int
client_main(int argc, char** argv);

TEST(test_cli, server_empty_start)
{
    testing_utils::make_a_secret();

    std::filesystem::create_directory("server_root");

    const char* args[] = {"server", "--root=server_root", "--secret=secret", "--timeout=1"};
    server_main(sizeof(args) / sizeof(char*), (char**)args);
}

TEST(test_cli, client_empty_start)
{
    testing_utils::make_a_secret();

    std::filesystem::create_directory("client_root");

    testing_utils::make_config("cfg", R"(
port=53881
ip=127.0.0.1
root=root
secret=config/secret
block_size=8096
number_of_loaders=8
blocks_in_batch=8


)");

    const char* args[] = {"client", "--root=client_root", "--secret=secret", "--config=cfg"};
    client_main(sizeof(args) / sizeof(char*), (char**)args);
}

TEST(test_cli, client_sync_all)
{
    testing_utils::make_a_secret();

    std::filesystem::create_directory("client_root");

    const char* args[] = {"client", "--root=client_root", "--secret=secret", "--interactive"};
    client_main(sizeof(args) / sizeof(char*), (char**)args);
}