#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "botan_all.h"

namespace llbridge
{
struct message_header;

class secure_session
{
    friend class secure_session_factory;

public:
    secure_session();

    void
    encrypt(message_header& mh, std::vector<uint8_t>& fcxt, std::vector<uint8_t>& dcxt);

    void
    decrypt(message_header& mh, std::vector<uint8_t>& fcxt, std::vector<uint8_t>& dcxt);

private:
    void
    encrypt_with_tag(uint64_t offset, uint64_t size, std::vector<uint8_t>& buffer);

    void
    decrypt_with_tag(uint64_t offset, uint64_t size, std::vector<uint8_t>& buffer);

    Botan::secure_vector<uint8_t> m_application_session_key;
    Botan::secure_vector<uint8_t> m_tag_buffer;
    Botan::System_RNG m_rng;

    std::unique_ptr<Botan::AEAD_Mode> m_encryption;
    std::unique_ptr<Botan::AEAD_Mode> m_decryption;
};

class secure_session_factory
{
public:
    static std::unique_ptr<secure_session_factory>
    create(const std::filesystem::path& secret) noexcept;

    std::unique_ptr<secure_session>
    create_session();

private:
    secure_session_factory()
        : m_application_session_key(16)
    {
    }

    Botan::secure_vector<uint8_t> m_application_session_key;
};

}  // namespace llbridge