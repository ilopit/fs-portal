#pragma once

#include <gtest/gtest.h>
#include <vector>

class file_read_cache_tf : public ::testing::Test
{
public:
    void
    SetUp();

    static void
    write_file();

    static std::vector<char>
    read_file();

    static const std::vector<char>&
    get_test_data();
};
