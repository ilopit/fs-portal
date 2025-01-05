

#include <gtest/gtest.h>

#include "engine/private/file_read_cache.h"

#include "test_helpers.h"

using namespace llbridge;

TEST_F(file_read_cache_tf, f_cache)
{
    file_cache fc;

    file_cache::open_config oc{
        .file = "bigfile", .chunk_size = 128, .cache_size = 16, .read_in_advance = 4};

    ASSERT_TRUE(fc.start(oc));

    auto noc = fc.number_of_chunks();

    std::vector<char> file_content;

    data_chunk_ptr ptr = nullptr;

    for (uint64_t i = 0; i < noc; ++i)
    {
        auto cr = fc.read_chunk(i);

        ASSERT_EQ(cr->id(), i);

        cr->read_data(ptr);

        for (auto c : ptr->data)
        {
            file_content.push_back((char)c);
        }
    }

    ASSERT_EQ(file_content, get_test_data());
}

TEST_F(file_read_cache_tf, f_cache_same_id)
{
    file_cache fc;

    file_cache::open_config oc{.file = "bigfile",
                               .chunk_size = get_test_data().size(),
                               .cache_size = 1,
                               .read_in_advance = 4};

    ASSERT_TRUE(fc.start(oc));

    auto noc = fc.number_of_chunks();

    std::vector<char> file_content;

    data_chunk_ptr ptr = nullptr;

    for (uint64_t i = 0; i < 5; ++i)
    {
        auto cr = fc.read_chunk(0);

        ASSERT_EQ(cr->id(), 0);

        cr->read_data(ptr);

        for (auto c : ptr->data)
        {
            file_content.push_back((char)c);
        }
        ASSERT_EQ(file_content, get_test_data());
        file_content.clear();
    }
}
