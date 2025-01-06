#include <engine/private/file.h>

#include <fcntl.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>

#if defined(WINDOWS)

#include <windows.h>
#include <io.h>

#define psx_open(path, flags) ::open(path, flags)
#define psx_close(fd) ::close(fd)
#define psx_write(fd, data, size) ::write(fd, data, size)
#define psx_seek(fd, offset, mode) ::_lseeki64(fd, offset, mode)

#else

#define psx_open(path, flags) ::open(path, flags)
#define psx_close(fd) ::close(fd)
#define psx_write(fd, data, size) ::write(fd, data, size)
#define psx_seek(fd, offset, mode) ::lseek64(fd, offset, mode)
#define psx_sync(fd) ::fsync(fd)

namespace
{
enum
{
    psx_binary = 0,
    psx_create = O_CREAT,
    psx_rdwd = O_RDWR
};
}
#endif

using namespace llbridge;

file::file(const std::filesystem::path& path)
    : m_handle(-1)
{
    (void)open(path);
}

file::~file()
{
    if (m_handle != -1)
    {
        flush();
        psx_close(m_handle);
        m_handle = -1;
    }
}

bool
file::is_open() const
{
    return m_handle != -1;
}

file::file()
    : m_handle(-1)
{
}

bool
file::open(const std::filesystem::path& path)
{
    if (m_handle != -1)
    {
        return false;
    }

    m_handle = psx_open(path.generic_string().c_str(), psx_binary | psx_create | psx_rdwd);

    return m_handle != -1;
}

uint64_t
file::write(const std::vector<uint8_t>& data)
{
    return psx_write(m_handle, data.data(), data.size());
}

bool
file::seek_to(uint64_t offset, int mode)
{
    return psx_seek(m_handle, offset, mode) != -1;
}

bool
file::flush()
{
    return psx_sync(m_handle) == 0;
}
