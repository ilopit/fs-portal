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

    const char* args[] = {"client", "--root=client_root", "--secret=secret"};
    client_main(sizeof(args) / sizeof(char*), (char**)args);
}

TEST(test_cli, client_sync_all)
{
    testing_utils::make_a_secret();

    std::filesystem::create_directory("client_root");

    const char* args[] = {"client", "--root=client_root", "--secret=secret", "--interactive"};
    client_main(sizeof(args) / sizeof(char*), (char**)args);
}