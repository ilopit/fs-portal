#include "engine/private/secure_session.h"

#include <engine/message.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include <fstream>
namespace llbridge
{

std::unique_ptr<secure_session_factory>
secure_session_factory::create(const std::filesystem::path& secret)
{
    std::ifstream secret_file(secret);

    if (!secret_file.is_open())
    {
        SPDLOG_ERROR("Secret doesn't exist at {}", secret.generic_string());
        return nullptr;
    }

    std::string v;

    std::getline(secret_file, v);
    auto master_key = Botan::hex_decode(v.data(), v.size());

    std::getline(secret_file, v);
    auto salt = Botan::hex_decode(v.data(), v.size());

#if defined(_DEBUG)
    constexpr size_t M = 16;
    constexpr size_t t = 1;
    constexpr size_t p = 1;
#else
    constexpr size_t M = 256 * 1024;
    constexpr size_t t = 4;
    constexpr size_t p = 2;
#endif

    auto pbkdf = Botan::PasswordHashFamily::create_or_throw("Argon2id")->from_params(M, t, p);
    BOTAN_ASSERT_NONNULL(pbkdf);

    std::string_view password;

    auto ssf = std::unique_ptr<secure_session_factory>();
    pbkdf->hash(ssf->m_application_session_key, password, salt);

    return ssf;
}

std::unique_ptr<secure_session>
secure_session_factory::create_session()
{
    auto obj = std::make_unique<secure_session>();

    obj->m_encryption =
        Botan::AEAD_Mode::create_or_throw("AES-128/GCM", Botan::Cipher_Dir::Encryption);

    obj->m_decryption =
        Botan::AEAD_Mode::create_or_throw("AES-128/GCM", Botan::Cipher_Dir::Decryption);

    obj->m_application_session_key = m_application_session_key;

    return obj;
}

secure_session::secure_session()
    : m_application_session_key(16)
{
}

void
secure_session::encrypt(message_header& mh, std::vector<uint8_t>& fctx, std::vector<uint8_t>& dctx)
{
    m_encryption->set_key(m_application_session_key);
    m_rng.randomize(mh.aead.data(), mh.aead.size());
    m_rng.randomize(mh.iv.data(), mh.iv.size());

    auto eaad_size = message_header::aead_size();
    m_encryption->set_associated_data(mh.aead_begin(), eaad_size);

    m_encryption->start(mh.iv.data(), mh.iv.size());

    if (mh.extra_blocks)
    {
        auto encypted_extra_size = mh.encypted_extra_size();
        if (dctx.size() < encypted_extra_size)
        {
            if (dctx.capacity() < encypted_extra_size)
            {
                SPDLOG_WARN("Performance hit! Resizing dctx from {} => {}", dctx.capacity(),
                            encypted_extra_size);
            }

            dctx.resize(encypted_extra_size);
        }

        m_encryption->process(std::span(fctx).subspan(constants::header_size()));

        encrypt_with_tag(0, encypted_extra_size, dctx);
    }
    else
    {
        encrypt_with_tag(constants::header_size(), message_header::encypted_fixed_size(), fctx);
    }
}

void
secure_session::decrypt(message_header& mh, std::vector<uint8_t>& fctx, std::vector<uint8_t>& dctx)
{
    m_decryption->set_key(m_application_session_key);
    m_decryption->set_associated_data(mh.aead_begin(), mh.aead_size());

    m_decryption->start(mh.iv.data(), constants::block_size());

    if (mh.extra_blocks)
    {
        if (dctx.size() < mh.encypted_extra_size())
        {
            if (dctx.capacity() < mh.encrypted_size())
            {
                SPDLOG_WARN("Performance hit! Resizing dctx with capacity {} => {}",
                            dctx.capacity(), mh.encypted_extra_size());
            }

            dctx.resize(mh.encypted_extra_size());
        }

        m_decryption->process(std::span(fctx).subspan(constants::header_size()));

        decrypt_with_tag(0, mh.encypted_extra_size(), dctx);
    }
    else
    {
        decrypt_with_tag(constants::header_size(), message_header::encypted_fixed_size(), fctx);
    }
}

void
secure_session::encrypt_with_tag(uint64_t offset, uint64_t size, std::vector<uint8_t>& buffer)
{
    m_encryption->process(
        std::span(buffer).subspan(offset, size - offset - constants::block_size()));

    m_tag_buffer.clear();
    m_encryption->finish(m_tag_buffer);
    std::memcpy(buffer.data() + size - constants::block_size(), m_tag_buffer.data(),
                constants::block_size());
}

void
secure_session::decrypt_with_tag(uint64_t offset, uint64_t size, std::vector<uint8_t>& buffer)
{
    m_decryption->process(
        std::span(buffer).subspan(offset, size - offset - constants::block_size()));

    m_tag_buffer.resize(constants::block_size());
    std::memcpy(m_tag_buffer.data(), buffer.data() + size - constants::block_size(),
                constants::block_size());
    m_decryption->finish(m_tag_buffer);
}

}  // namespace llbridge