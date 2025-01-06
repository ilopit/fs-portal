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
        bool
        update_from_config(const std::filesystem::path& p);

        void
        print();

        std::string port = "53881";
        std::string ip = "127.0.0.1";
        std::filesystem::path root = std::filesystem::current_path() / "root";
        std::filesystem::path secret = std::filesystem::current_path() / "config/secret";
        uint64_t block_size = 1024U * 1024U * 1U;
        uint64_t number_of_loaders = 8U;
        uint64_t blocks_in_batch = 8U;
    };

    static client
    make(config cfg);

    client();
    ~client();

    bool
    open();

    const file_list&
    list();

    void
    print_list();

    bool
    sync_all();

    bool
    sync(uint64_t file_id);

private:
    client(std::unique_ptr<client_impl> impl);

    std::unique_ptr<client_impl> m_impl;
};

}  // namespace llbridge
