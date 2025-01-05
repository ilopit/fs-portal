#include <gtest/gtest.h>

#include "engine/private/binary_stream.h"

using namespace llbridge;

TEST(test_binary_stream, all)
{
    constexpr uint64_t total_size = 40;
    std::vector<uint8_t> t(total_size, 0u);
    auto bos = binary_stream::from_vector(t);

    std::string value1 = "555555";
    bos.write(value1);
    std::string value2 = "11111";
    bos.write(value2);

    auto written_size = bos.cursor - bos.begin;

    ASSERT_EQ(value1.size() + value2.size() + sizeof(uint64_t) * 2, written_size);

    for (; written_size < total_size; ++written_size)
    {
        ASSERT_EQ(t[written_size], 0);
    }
}