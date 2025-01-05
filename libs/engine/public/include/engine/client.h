#pragma once

#include <string>
#include <memory>

#include <filesystem>
#include <engine/files_list.h>

namespace llbridge
{
class client_impl;

class client
{
public:
    struct config
    {
        std::string port = "5050";
        std::string ip = "127.0.0.1";
        std::filesystem::path root = std::filesystem::current_path() / "root";
        std::filesystem::path secret = std::filesystem::current_path() / "config/secret";
    };

    static client
    make(const config& cfg);

    client();
    ~client();

    bool
    open();

    const file_list&
    list();

    bool
    sync_all();

    bool
    sync(uint64_t file_id);

    client(std::unique_ptr<client_impl> impl);

private:
    std::unique_ptr<client_impl> m_impl;
};

}  // namespace llbridge
