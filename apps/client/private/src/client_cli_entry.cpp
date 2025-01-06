#include <engine/client.h>

#include <spdlog/spdlog.h>
#include <iostream>
#include <string>
#include <optional>

using namespace llbridge;

int
client_main(int argc, char** argv)
{
    client::config cfg;

    bool is_interective = false;
    bool run_list = false;
    std::optional<int> sync_value;

    for (int i = 1; i < argc; ++i)
    {
        std::string value = argv[i];

        if (value.starts_with("--config="))
        {
            std::string path = value.substr(sizeof("--config=") - 1);
            if (!cfg.update_from_config(path))
            {
                SPDLOG_ERROR("Failed to read config at {}", path);
                return -1;
            }
        }
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string value = argv[i];

        if (value.starts_with("--config="))
        {
            continue;
        }
        else if (value.starts_with("--port="))
        {
            cfg.port = value.substr(sizeof("--port=") - 1);
        }
        else if (value.starts_with("--ip="))
        {
            cfg.ip = value.substr(sizeof("--ip=") - 1);
        }
        else if (value.starts_with("--root="))
        {
            cfg.root = value.substr(sizeof("--root=") - 1);
        }
        else if (value.starts_with("--secret="))
        {
            cfg.secret = value.substr(sizeof("--secret=") - 1);
        }
        else if (value.starts_with("--interactive"))
        {
            is_interective = true;
        }
        else if (value.starts_with("--sync="))
        {
            auto sync_tmp = value.substr(sizeof("--sync=") - 1);
            if (sync_tmp == "all")
            {
                sync_value = -1;
            }
            else
            {
                sync_value = std::stoi(sync_tmp);
            }
        }
        else
        {
            SPDLOG_ERROR("Unsupported argument {}", value);
            return -1;
        }
    }

    auto c = client::make(cfg);

    if (!c.open())
    {
        SPDLOG_ERROR("Failed to open client");
        return -1;
    }

    if ((is_interective && sync_value.has_value()) || (is_interective && run_list))
    {
        SPDLOG_ERROR("Unsupported commands");
        return -1;
    }

    if (is_interective)
    {
        for (;;)
        {
            std::string s;
            std::getline(std::cin, s);

            if (s == "exit")
            {
                break;
            }
            else if (s == "ls")
            {
                c.print_list();
            }
            else if (s.starts_with("sync "))
            {
                int id = std::stoi(s.substr(sizeof("sync") - 1));

                c.sync(id);
            }
            else
            {
                SPDLOG_ERROR("Unsupported command [{}]", s);
            }
        }
    }

    return 0;
}