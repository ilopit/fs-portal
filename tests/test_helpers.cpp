#include "test_helpers.h"

#include <filesystem>
#include <fstream>

void
file_read_cache_tf::SetUp()
{
    std::error_code ec = {};
    std::filesystem::remove("bifgile", ec);
    (void)ec;

    std::filesystem::remove((std::filesystem::absolute("output") / "bigfile"), ec);
    (void)ec;

    write_file();
}

void
file_read_cache_tf::write_file()
{
    std::ofstream file("bigfile", std::ios_base::binary);
    file.write((char*)get_test_data().data(), get_test_data().size());
}

std::vector<char>
file_read_cache_tf::read_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);

    if (!file.is_open())
    {
        EXPECT_TRUE(false);
    }

    uint64_t size = file.tellg();

    file.seekg(0, std::ios_base::beg);
    std::vector<char> result(size, '\0');

    file.read(result.data(), size);

    auto gc = file.gcount();

    return result;
}

const std::vector<char>&
file_read_cache_tf::get_test_data()
{
    static const std::vector<char> TEST_DATA = []()
    {
        std::vector<char> data;
        for (int i = 0; i < 17; ++i)
            for (int j = 0; j < 7; ++j)
            {
                data.push_back('a' + i);
            }
        return data;
    }();

    return TEST_DATA;
}
