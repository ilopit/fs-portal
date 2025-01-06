#include <engine/private/file.h>

#include <stdio.h>
#include <fcntl.h>
#include <windows.h>
#include <io.h>

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
        _close(m_handle);
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

    m_handle = ::_wopen(path.generic_wstring().c_str(), _O_BINARY | _O_CREAT | _O_RDWR);
    // _wfopen(path.generic_wstring().c_str(), L"rb+");

    return m_handle != -1;
}

uint64_t
file::write(const std::vector<uint8_t>& data)
{
    return _write(m_handle, data.data(), data.size());
}

bool
file::seek_to(uint64_t offset, int mode)
{
    return _lseeki64(m_handle, offset, mode) != -1;
}

bool
file::flush()
{
    return _commit(m_handle) == 0;
}
