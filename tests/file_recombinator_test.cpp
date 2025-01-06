#include <gtest/gtest.h>

#include "engine/private/file_recombinator.h"
#include "test_helpers.h"
#include <algorithm>
#include <random>

#include <spdlog/fmt/bin_to_hex.h>

using namespace llbridge;

class file_recombinator_tf : public file_read_cache_tf
{
};

TEST_F(file_recombinator_tf, happy_path)
{
    file_recombinator fc;

    file_recombinator::open_config oc{
        .file = std::filesystem::absolute("output") / "bigfile",
        .chunk_size = 32,
        .file_size = get_test_data().size(),
        .batch_size = 4,
    };

    ASSERT_TRUE(fc.start(oc));

    uint64_t idx = 0;

    std::vector<data_chunk_ptr> chunks;

    for (size_t i = 0; i < get_test_data().size(); i += oc.chunk_size)
    {
        auto last = std::min(get_test_data().size(), i + oc.chunk_size);

        std::vector<std::uint8_t> bb(get_test_data().data() + i, get_test_data().data() + last);

        chunks.emplace_back(std::make_unique<data_chunk>(idx++, bb));
    }

    std::mt19937 g;
    g.seed();
    std::shuffle(chunks.begin(), chunks.end(), g);

    std::vector<write_result_ptr> results;
    for (auto& c : chunks)
    {
        results.emplace_back(fc.write_chunk(std::move(c)));
    }

    for (auto& r : results)
    {
        r->wait();
    }

    fc.stop();

    auto vv = read_file(std::filesystem::absolute("output") / "bigfile");

    SPDLOG_INFO("{:spn}", spdlog::to_hex(get_test_data()));
    SPDLOG_INFO("{:spn}", spdlog::to_hex(vv));

    ASSERT_EQ(vv.size(), get_test_data().size());
    for (auto i = 0; i < vv.size(); ++i)
    {
        ASSERT_EQ(vv[i], get_test_data()[i]) << i;
    }
}
