#include <engine/files_list.h>

#include <spdlog/spdlog.h>
namespace llbridge
{

file_list
file_list::init(const std::filesystem::path& root_dir)
{
    file_list result = {};

    for (const std::filesystem::directory_entry& dir_entry :
         std::filesystem::recursive_directory_iterator(root_dir))
    {
        if (!dir_entry.is_regular_file())
        {
            continue;
        }

        auto rp = std::filesystem::relative(dir_entry, root_dir);

        file_description fd{.path = rp, .size = dir_entry.file_size()};

        result.list.push_back(fd);

        std::sort(result.list.begin(), result.list.end());
    }

    return result;
}

void
file_list::print()
{
    int id = 0;
    for (auto& f : list)
    {
        SPDLOG_INFO("{:<5} | {:<30} | {:<10}", id++, f.path.generic_string(), f.size);
    }
}

}  // namespace llbridge