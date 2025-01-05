#pragma once

#include <vector>
#include <string>

namespace llbridge
{

struct binary_stream
{
    static binary_stream
    from_vector(std::vector<std::uint8_t>& buffer)
    {
        return {
            .cursor = buffer.data(), .begin = buffer.data(), .end = buffer.data() + buffer.size()};
    }

    void
    reset()
    {
        cursor = begin;
    }

    template <typename T>
    void
    write(const T& data)
    {
        memcpy(cursor, &data, sizeof(T));
        skip_field(data);
    }

    void
    write(const std::string& str);

    void
    write(void* data, uint64_t size)
    {
        memcpy(cursor, data, size);
        skip(size);
    }

    template <typename T>
    void
    read(T& data)
    {
        memcpy(&data, cursor, sizeof(T));
        skip_field(data);
    }

    void
    read(void* data, uint64_t size)
    {
        memcpy(data, cursor, size);
        skip(size);
    }

    void
    read(std::string& str);

    template <typename T>
    void
    skip_field(T&)
    {
        cursor += sizeof(T);
    }

    template <typename T>
    void
    skip_field()
    {
        cursor += sizeof(T);
    }

    void
    skip(size_t size)
    {
        cursor += size;
    }

    std::uint8_t* cursor = nullptr;
    std::uint8_t* begin = nullptr;
    const std::uint8_t* end = nullptr;
};

}  // namespace llbridge