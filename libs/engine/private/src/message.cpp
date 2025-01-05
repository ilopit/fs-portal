#include "engine/message.h"

#include <fstream>

#include <spdlog/spdlog.h>

#include <engine/private/binary_stream.h>

#include <spdlog/fmt/bin_to_hex.h>

namespace llbridge
{

uint64_t
list_message_extra_part::extract(std::vector<std::uint8_t>& extra_content, uint64_t exra_size)
{
    auto c = binary_stream::from_vector(extra_content);

    uint64_t number_of_files = 0;
    c.read(number_of_files);

    for (int i = 0; i < number_of_files; ++i)
    {
        file_description fd;
        c.read(fd.size);
        std::string path;
        c.read(path);
        fd.path = path;
        files.list.push_back(fd);
    }

    return 0;
}

uint64_t
list_message_extra_part::pack(std::vector<std::uint8_t>& extra_content)
{
    auto c = binary_stream::from_vector(extra_content);

    uint64_t size = files.list.size();
    c.write(size);

    for (auto& f : files.list)
    {
        c.write(f.size);
        std::string path = f.path.string();
        c.write(path);
    }

    return c.cursor - c.begin;
}

uint64_t
error_message_extra_part::extract(std::vector<std::uint8_t>& v, uint64_t exra_siz)
{
    auto c = binary_stream::from_vector(v);

    c.read(details);
    return c.cursor - c.begin;
}

uint64_t
error_message_extra_part::pack(std::vector<std::uint8_t>& v)
{
    auto c = binary_stream::from_vector(v);

    c.write(details);

    return c.cursor - c.begin;
}

uint64_t
chunk_response_extra_part::extract(std::vector<std::uint8_t>& v, uint64_t exra_size)
{
    data = std::move(v);

    data.resize(exra_size);

    return true;
}

uint64_t
chunk_response_extra_part::pack(std::vector<std::uint8_t>& v)
{
    v = std::move(data);

    return v.size();
}

void
message_header::print(spdlog::level::level_enum print_level)
{
    SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), print_level,
                       "[{}]\n      iv: {:spn}\n    aead: {:spn}\n  blocks: {}\n   extra: {}\n"
                       " version: {}\n type:  {}",
                       (uint64_t)this, spdlog::to_hex(iv), spdlog::to_hex(aead), extra_blocks,
                       extra, version, (int)type);
}

}  // namespace llbridge