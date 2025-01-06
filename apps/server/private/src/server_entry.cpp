#include <engine/server.h>

#include <spdlog/spdlog.h>

using namespace llbridge;

int
server_main(int argc, char** argv)
{
    server::config cfg;
    uint64_t timeout = 0;

    for (int i = 1; i < argc; ++i)
    {
        std::string value = argv[i];

        if (value.starts_with("--port="))
        {
            cfg.port = value.substr(sizeof("--port=") - 1);
        }
        else if (value.starts_with("--root="))
        {
            cfg.root = value.substr(sizeof("--root=") - 1);
        }
        else if (value.starts_with("--secret="))
        {
            cfg.secret = value.substr(sizeof("--secret=") - 1);
        }
        else if (value.starts_with("--timeout="))
        {
            timeout = std::stoi(value.substr(sizeof("--timeout=") - 1));
        }
        else
        {
            SPDLOG_ERROR("Unsupported arg [{}]", value);
            return -1;
        }
    }

    auto c = server::make(cfg);

    c.start();

    SPDLOG_INFO("Server will run for {} second ...", timeout);
    std::this_thread::sleep_for(std::chrono::seconds{timeout});
    SPDLOG_INFO("Stopping ...");
    c.stop();

    return 0;
}