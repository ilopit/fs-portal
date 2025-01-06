#pragma once

#include <memory>
#include <filesystem>

#include <functional>

namespace llbridge
{

class server_impl;

class server
{
public:
    server();
    ~server();

    using sessin_cb = std::function<void(uint64_t)>;

    server&
    operator=(server&) = delete;

    struct config
    {
        bool
        update_from_config(const std::filesystem::path& p);

        void
        print();

        std::string port = "53881";
        std::filesystem::path root = std::filesystem::current_path() / "root";
        std::filesystem::path secret = std::filesystem::current_path() / "secret";
    };

    static server
    make(config cfg);

    bool
    start();

    bool
    stop();

private:
    void
    start_impl();

    server(std::shared_ptr<server_impl> impl);

    std::shared_ptr<server_impl> m_impl;
};

}  // namespace llbridge