#pragma once

#include <filesystem>
#include <set>
#include <vector>

namespace llbridge
{
struct file_description
{
    std::filesystem::path path;
    uint64_t size = 0;

    bool
    operator<(const file_description& other) const
    {
        return path < other.path;
    }
    bool
    operator==(const file_description& other) const
    {
        return std::tie(path, size) == std::tie(other.path, other.size);
    }
};

struct file_list
{
    static file_list
    init(const std::filesystem::path& root);

    std::vector<file_description> list;

    void
    print();

    bool
    operator==(const file_list& other) const
    {
        return list == other.list;
    }
};

}  // namespace llbridge