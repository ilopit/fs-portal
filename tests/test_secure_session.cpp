#include <engine/private/secure_session.h>
#include <engine/message.h>
#include <fstream>

#include "botan_all.h"

#include <gtest/gtest.h>

using namespace llbridge;

constexpr auto secret_file = "secret";
void
make_a_secret()
{
    std::ofstream fs(secret_file);

    fs << "01234567890ABCDEF" << std::endl;
    fs << "FEDCBA09876543210" << std::endl;
}

TEST(test_encryption, fixed)
{
    make_a_secret();

    auto ec = secure_session_factory::create(secret_file)->create_session();

    message_header mh{};

    std::vector<uint8_t> fctx;
    fctx.resize(message_header::encypted_fixed_size(), 33u);

    std::vector<uint8_t> dctx;

    ec->encrypt(mh, fctx, dctx);
    ASSERT_EQ(fctx.size(), message_header::encypted_fixed_size());
    ASSERT_EQ(dctx.size(), 0);

    for (uint64_t i = 0; i < constants::header_size(); ++i)
    {
        ASSERT_EQ(fctx[i], 33u) << "failed at " << i;
    }

    for (uint64_t i = constants::header_size() / 16; i < fctx.size() / 16 - 1; ++i)
    {
        int pcount = 0;
        for (uint64_t j = i * 16; j < (i + 1) * 16; ++j)
        {
            pcount = fctx[j] == 33u ? pcount + 1 : pcount;
        }
        ASSERT_LE(pcount, 4);
    }

    ec->decrypt(mh, fctx, dctx);
    ASSERT_EQ(fctx.size(), message_header::encypted_fixed_size());
    ASSERT_EQ(dctx.size(), 0);

    for (uint64_t i = 0; i < fctx.size() - constants::block_size(); ++i)
    {
        ASSERT_EQ(fctx[i], 33u) << "failed at " << i;
    }
}

TEST(test_encryption, dynamic)
{
    make_a_secret();

    auto ec = secure_session_factory::create(secret_file)->create_session();

    message_header mh{};

    std::vector<uint8_t> fctx(message_header::encypted_fixed_size(), 33u);

    constexpr uint64_t k_dynamic_buffer_size = 127U;

    mh.set_extra_size(k_dynamic_buffer_size);

    std::vector<uint8_t> dctx(mh.encypted_extra_size(), 11u);

    ec->encrypt(mh, fctx, dctx);
    ASSERT_EQ(fctx.size(), message_header::encypted_fixed_size());
    ASSERT_EQ(dctx.size(), mh.encypted_extra_size());

    for (uint64_t i = constants::header_size() / 16; i < fctx.size() / 16; ++i)
    {
        int pcount = 0;
        for (uint64_t j = i * 16; j < (i + 1) * 16; ++j)
        {
            pcount = fctx[j] == 33u ? pcount + 1 : pcount;
        }
        ASSERT_LE(pcount, 4);
    }

    for (uint64_t i = 0; i < fctx.size() / 16 - 1; ++i)
    {
        int pcount = 0;
        for (uint64_t j = i * 16; j < (i + 1) * 16; ++j)
        {
            pcount = fctx[j] == 11u ? pcount + 1 : pcount;
        }
        ASSERT_LE(pcount, 4);
    }

    ec->decrypt(mh, fctx, dctx);
    ASSERT_EQ(fctx.size(), message_header::encypted_fixed_size());
    ASSERT_EQ(dctx.size(), mh.encypted_extra_size());

    for (uint64_t i = 0; i < fctx.size() - constants::header_size(); ++i)
    {
        ASSERT_EQ(fctx[i], 33u) << "failed at " << i;
    }

    for (uint64_t i = 0; i < dctx.size() - constants::header_size(); ++i)
    {
        ASSERT_EQ(dctx[i], 11u) << "failed at " << i;
    }
}