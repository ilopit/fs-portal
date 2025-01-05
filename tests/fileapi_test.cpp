
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace
{
const std::vector<char> TEST_DATA = []()
{
    std::vector<char> data;
    for (int i = 0; i < 21; ++i)
        for (int i = 0; i < 21; ++i)
        {
            data.push_back('a' + i);
        }
    return data;
}();

class file_read_tf : public ::testing::Test
{
public:
    void
    SetUp()
    {
        std::error_code ec = {};
        std::filesystem::remove("bifgile", ec);
        (void)ec;
    }
};

void
write_file()
{
    std::ofstream file("bigfile", std::ios_base::binary);
    file.write((char*)TEST_DATA.data(), TEST_DATA.size());
}

std::vector<char>
read_file()
{
    std::ifstream file("bigfile", std::ios_base::binary | std::ios_base::ate);

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

}  // namespace

TEST_F(file_read_tf, read)
{
    //     write_file();
    //
    //     file_api fapi;
    //
    //     uint64_t i = 0;
    //     file_api::read_config rc{.file = "bigfile", .buffer_size = 128, .buffers_count = 16};
    //
    //     fapi.run(rc);
    //
    //     std::vector<char> file_content;
    //
    //     std::unique_ptr<buffer> b = nullptr;
    //     std::vector<std::byte> swap_buf(1);
    //
    //     const auto number_of_buffers = fapi.buffers_count();
    //
    //     for (;;)
    //     {
    //         const bool is_last = i == (number_of_buffers - 1);
    //
    //         const auto r = fapi.get_read_buffer(b);
    //
    //         ASSERT_NE(r, file_api::result::no_buffers);
    //
    //         if (r == file_api::result::not_ready)
    //         {
    //             std::this_thread::sleep_for(std::chrono::milliseconds{10});
    //             continue;
    //         }
    //         else if (r == file_api::result::end)
    //         {
    //             break;
    //         }
    //
    //         ASSERT_TRUE(b);
    //
    //         b->swap(swap_buf);
    //
    //         ASSERT_EQ(b->offset(), i * rc.buffer_size);
    //
    //         const uint64_t size = is_last ? TEST_DATA.size() % rc.buffer_size : rc.buffer_size;
    //
    //         ASSERT_EQ(swap_buf.size(), size);
    //
    //         for (auto c : swap_buf)
    //         {
    //             file_content.push_back((char)c);
    //         }
    //
    //         fapi.release_read_buffer(std::move(b));
    //         ++i;
    //     }
    //
    //     ASSERT_EQ(file_content, TEST_DATA);
}

TEST_F(file_read_tf, read_batch)
{
    //     write_file();
    //
    //     uint64_t i = 0;
    //
    //     file_api::read_config rc{.file = "bigfile", .buffer_size = 128, .buffers_count = 16};
    //
    //     file_api fapi;
    //     fapi.run(rc);
    //
    //     std::vector<char> file_content;
    //
    //     std::vector<std::byte> swap_buf(1);
    //
    //     const auto number_of_buffers = fapi.buffers_count();
    //
    //     std::vector<std::unique_ptr<buffer>> buffers(number_of_buffers);
    //
    //     for (;;)
    //     {
    //         const bool is_last = i == number_of_buffers - 1;
    //
    //         const auto r = fapi.get_read_buffer(buffers[i % number_of_buffers]);
    //
    //         if (r == file_api::result::not_ready)
    //         {
    //             std::this_thread::sleep_for(std::chrono::milliseconds{10});
    //             continue;
    //         }
    //         else if (r == file_api::result::end)
    //         {
    //             break;
    //         }
    //         else if (r == file_api::result::no_buffers)
    //         {
    //             for (auto& b : buffers)
    //             {
    //                 fapi.release_read_buffer(std::move(b));
    //             }
    //             continue;
    //         }
    //         auto& b = buffers[i % number_of_buffers];
    //
    //         ASSERT_TRUE(b);
    //
    //         ASSERT_EQ(b->offset(), i * rc.buffer_size);
    //
    //         b->swap(swap_buf);
    //
    //         const uint64_t size = is_last ? TEST_DATA.size() % rc.buffer_size : rc.buffer_size;
    //
    //         ASSERT_EQ(swap_buf.size(), size);
    //
    //         for (auto c : swap_buf)
    //         {
    //             file_content.push_back((char)c);
    //         }
    //
    //         ++i;
    //     }
    //
    //     ASSERT_EQ(file_content, TEST_DATA);
}

TEST_F(file_read_tf, write)
{
    //     file_api fapi;
    //
    //     uint64_t i = 0;
    //
    //     file_api::write_config wc{
    //         .file = "bigfile", .buffer_size = 128, .buffers_count = 16, .file_size =
    //         TEST_DATA.size()};
    //     fapi.run(wc);
    //
    //     std::unique_ptr<buffer> b = nullptr;
    //     std::vector<std::byte> swap_buf(wc.buffer_size);
    //
    //     const auto number_of_buffers = fapi.buffers_count();
    //
    //     for (;;)
    //     {
    //         const auto r = fapi.get_write_buffer(b);
    //
    //         if (r == file_api::result::not_ready || r == file_api::result::no_buffers)
    //         {
    //             std::this_thread::sleep_for(std::chrono::milliseconds{10});
    //             continue;
    //         }
    //         else if (r == file_api::result::end)
    //         {
    //             break;
    //         }
    //
    //         ASSERT_TRUE(b);
    //         b->set_offset(i);
    //
    //         swap_buf.clear();
    //         for (uint64_t j = 0; j < wc.buffer_size && i < TEST_DATA.size(); ++j, ++i)
    //         {
    //             swap_buf.push_back((std::byte)TEST_DATA[i]);
    //         }
    //
    //         b->swap(swap_buf);
    //
    //         fapi.release_write_buffer(std::move(b));
    //     }
    //
    //     auto file_content = read_file();
    //     ASSERT_EQ(file_content, TEST_DATA);
}

TEST_F(file_read_tf, write_batch)
{
    //     file_api fapi;
    //
    //     uint64_t i = 0;
    //
    //     file_api::write_config wc{
    //         .file = "bigfile", .buffer_size = 128, .buffers_count = 16, .file_size =
    //         TEST_DATA.size()};
    //     fapi.run(wc);
    //
    //     std::vector<std::byte> swap_buf(wc.buffer_size);
    //
    //     const auto number_of_buffers = fapi.buffers_count();
    //
    //     std::vector<std::unique_ptr<buffer>> buffers(number_of_buffers);
    //
    //     for (;;)
    //     {
    //         const auto r = fapi.get_write_buffer(buffers[i % number_of_buffers]);
    //
    //         if (r == file_api::result::not_ready)
    //         {
    //             std::this_thread::sleep_for(std::chrono::milliseconds{10});
    //             continue;
    //         }
    //         else if (r == file_api::result::end)
    //         {
    //             break;
    //         }
    //         else if (r == file_api::result::no_buffers)
    //         {
    //             for (auto& b : buffers)
    //             {
    //                 fapi.release_write_buffer(std::move(b));
    //             }
    //             continue;
    //         }
    //
    //         auto& b = buffers[i % number_of_buffers];
    //
    //         ASSERT_TRUE(b);
    //         b->set_offset(i * wc.buffer_size);
    //
    //         swap_buf.clear();
    //         for (uint64_t j = 0; j < wc.buffer_size && (i * wc.buffer_size + j) <
    //         TEST_DATA.size(); ++j)
    //         {
    //             swap_buf.push_back((std::byte)TEST_DATA[i * wc.buffer_size + j]);
    //         }
    //
    //         b->swap(swap_buf);
    //
    //         ++i;
    //     }
    //
    //     auto file_content = read_file();
    //     ASSERT_EQ(file_content, TEST_DATA);
}

TEST_F(file_read_tf, write_batch_lock)
{
    //     file_api fapi;
    //
    //     uint64_t i = 0;
    //
    //     file_api::write_config wc{
    //         .file = "bigfile", .buffer_size = 128, .buffers_count = 2, .file_size =
    //         TEST_DATA.size()};
    //     fapi.run(wc);
    //
    //     std::vector<std::byte> swap_buf(wc.buffer_size);
    //
    //     const auto number_of_buffers = fapi.buffers_count();
    //
    //     std::vector<std::unique_ptr<buffer>> buffers(number_of_buffers);
    //
    //     for (;;)
    //     {
    //         auto r = fapi.get_write_buffer(buffers[i % number_of_buffers]);
    //
    //         if (r == file_api::result::not_ready)
    //         {
    //             std::this_thread::sleep_for(std::chrono::milliseconds{10});
    //             continue;
    //         }
    //         else if (r == file_api::result::no_buffers)
    //         {
    //             for (auto& b : buffers)
    //             {
    //                 fapi.release_write_buffer(std::move(b));
    //             }
    //             ++i;
    //             r = fapi.get_write_buffer(buffers[i % number_of_buffers]);
    //             ASSERT_EQ(r, file_api::result::locked);
    //             return;
    //         }
    //
    //         auto& b = buffers[i % number_of_buffers];
    //
    //         ASSERT_TRUE(b);
    //         b->set_offset((i + 1) * wc.buffer_size);
    //
    //         swap_buf.clear();
    //         for (uint64_t j = 0; j < wc.buffer_size && (i * wc.buffer_size + j) <
    //         TEST_DATA.size(); ++j)
    //         {
    //             swap_buf.push_back((std::byte)TEST_DATA[i * wc.buffer_size + j]);
    //         }
    //
    //         b->swap(swap_buf);
    //
    //         ++i;
    //     }
}
