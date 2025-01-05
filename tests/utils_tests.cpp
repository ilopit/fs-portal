#include <gtest/gtest.h>

#include "engine/private/utils.h"
#include "engine/private/client_transport_context.h"
#include "engine/private/server_context.h"
#include "engine/message.h"
#include "engine/communication_context.h"

using namespace llbridge;

TEST(utils, DISABLED_happy_path)
{
    llbridge::utils::file_competition fc;

    fc.init(950, 1000);

    ASSERT_EQ(fc.m_chunk_size, 1000);
    ASSERT_EQ(fc.m_file_size, 950);
    ASSERT_EQ(fc.m_mapping.size(), 1);

    fc.init(1050, 1000);

    ASSERT_EQ(fc.m_chunk_size, 1000);
    ASSERT_EQ(fc.m_file_size, 1050);
    ASSERT_EQ(fc.m_mapping.size(), 2);

    utils::rt_event er;

    std::thread t(
        [&er]()
        {
            std::this_thread::sleep_for(std::chrono::seconds{5});
            er.signal();
        });

    er.wait();
    t.join();
}

TEST(utils, test_messages)
{
}
