#pragma once

#include <gtest/gtest.h>

#include <vector>
#include <filesystem>

class file_read_cache_tf : public ::testing::Test
{
public:
    void
    SetUp();

    static void
    write_file();

    static std::vector<char>
    read_file(const std::filesystem::path& path);

    static const std::vector<char>&
    get_test_data();
};
