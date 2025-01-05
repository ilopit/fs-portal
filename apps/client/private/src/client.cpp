#include <engine/client.h>

using namespace llbridge;

int
main(int argc, char** argv)
{
    client::config cfg;

    for (int i = 1; i < argc; ++i)
    {
        std::string value = argv[i];

        if (value.starts_with("--port="))
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
        else
        {
        }
    }

    auto c = client::make(cfg);

    return 0;
}