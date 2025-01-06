#include <gtest/gtest.h>

#include <engine/message_bridge.h>
#include <engine/private/secure_session.h>

#include <fstream>

using namespace llbridge;

constexpr auto secret_file = "secret";
static void
make_a_secret()
{
    std::ofstream fs(secret_file);

    fs << "0123456789ABCDEF" << std::endl;
    fs << "FEDCBA9876543210" << std::endl;
}

TEST(test_messages, header)
{
    message_header mh;

    mh.set_extra_size(15);

    ASSERT_EQ(mh.extra_blocks, 1);
    ASSERT_EQ(mh.extra, 15);

    ASSERT_EQ(mh.get_extra_size(), 15);
}

TEST(test_messages, all)
{
    make_a_secret();

    auto ssf = secure_session_factory::create(secret_file);

    {
        message_bridge mh(ssf->create_session());
        ASSERT_TRUE(mh.write(download_file_session_request().make_fixed(11, 22, 33)));

        download_file_session_request receive_msg;
        ASSERT_TRUE(mh.read(receive_msg));

        ASSERT_EQ(receive_msg.fixed.id, 11);
        ASSERT_EQ(receive_msg.fixed.chunk_size, 22);
        ASSERT_EQ(receive_msg.fixed.offset, 33);
    }

    //     {
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(error_message().extra_part("25").fixed_part(25)));
    //
    //         error_message receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.extra.details, "25");
    //         ASSERT_EQ(receive_msg.fixed.ec, 25);
    //    }

    //     {
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(download_file_session_response().fixed_part(11, 22)));
    //
    //         download_file_session_response receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.fixed.session_id, 11);
    //         ASSERT_EQ(receive_msg.fixed.number_of_chunks, 22);
    //     }
    //     {
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(download_file_session_end().fixed_part(11)));
    //
    //         download_file_session_end receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.fixed.session_id, 11);
    //     }
    //     {
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(chunk_request().fixed_part(11, 22)));
    //
    //         chunk_request receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.fixed.session_id, 11);
    //         ASSERT_EQ(receive_msg.fixed.chunk_id, 22);
    //     }
    //     {
    //         auto expected = std::vector<std::uint8_t>(333, 0u);
    //
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(chunk_response().fixed_part(11, 22).extra_part(expected)));
    //
    //         chunk_response receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.fixed.session_id, 11);
    //         ASSERT_EQ(receive_msg.fixed.chunk_id, 22);
    //         ASSERT_EQ(receive_msg.extra.data, expected);
    //     }
    //     {
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(hello_request().fixed_part()));
    //
    //         hello_request receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //     }
    //     {
    //         file_list fl{.list = {{"a", 11}, {"b", 22}}};
    //
    //         message_bridge mh(1024);
    //         ASSERT_TRUE(mh.write(list_message().extra_part(fl)));
    //
    //         list_message receive_msg;
    //         ASSERT_TRUE(mh.read(receive_msg));
    //
    //         ASSERT_EQ(receive_msg.extra.files, fl);
    //     }
}
