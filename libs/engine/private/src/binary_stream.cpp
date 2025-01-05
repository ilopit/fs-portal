#include <engine/private/binary_stream.h>

namespace llbridge
{

void
binary_stream::write(const std::string& str)
{
    uint64_t size = str.size();
    write(size);
    write((std::byte*)str.data(), size);
}

void
binary_stream::read(std::string& str)
{
    uint64_t size = 0U;
    read(size);
    str.assign((const char*)cursor, size);
    skip(size);
}
}  // namespace llbridge