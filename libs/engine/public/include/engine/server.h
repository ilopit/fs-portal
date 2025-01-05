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
        std::string port = "53881";
        std::filesystem::path root_dir = std::filesystem::current_path() / "files";
        std::filesystem::path secret_file = std::filesystem::current_path() / "secret";
    };

    static server
    make(const config& cfg);

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