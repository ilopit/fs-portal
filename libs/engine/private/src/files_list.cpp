#include <engine/files_list.h>

#include <spdlog/spdlog.h>
#include <iostream>

namespace llbridge
{

bool
file_list::init(const std::filesystem::path& root, file_list& fl)
{
    fl = {};

    if (!std::filesystem::is_directory(root))
    {
        SPDLOG_ERROR("Root [{}] doesn't exist", root.generic_string());
        return false;
    }
    for (const std::filesystem::directory_entry& dir_entry :
         std::filesystem::recursive_directory_iterator(root))
    {
        if (!dir_entry.is_regular_file())
        {
            continue;
        }

        auto rp = std::filesystem::relative(dir_entry, root);

        file_description fd{.path = rp, .size = dir_entry.file_size()};

        fl.list.push_back(fd);

        std::sort(fl.list.begin(), fl.list.end());
    }

    return true;
}

void
file_list::log_print()
{
    int id = 0;
    for (auto& f : list)
    {
        SPDLOG_INFO("{:<5} | {:<30} | {:<10}", id++, f.path.generic_string(), f.size);
    }
}

void
file_list::print()
{
    int id = 0;

    for (auto& f : list)
    {
        std::cout << std::format("{:<5} | {:<30} | {:<10}", id++, f.path.generic_string(), f.size)
                  << std::endl;
    }
}

}  // namespace llbridge