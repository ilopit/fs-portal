#pragma once

#include <filesystem>
#include <vector>

namespace llbridge
{
class psx_file
{
public:
    psx_file();

    ~psx_file();

    psx_file(const std::filesystem::path& path);

    bool
    is_open() const;

    bool
    open(const std::filesystem::path& path);

    uint64_t
    write(const std::vector<uint8_t>& data);

    bool
    seek_to(uint64_t offset, int mode);

    bool
    flush();

public:
    // struct ::FILE* m_handle;
    int m_handle = -1;
};

}  // namespace llbridge