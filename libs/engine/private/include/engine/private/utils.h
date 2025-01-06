#pragma once

#include <vector>
#include <array>
#include <bitset>
#include <condition_variable>
#include <mutex>

#include <boost/asio.hpp>

#include "engine/message.h"

namespace llbridge
{
void
trim_eols(std::string& str);

struct data_chunk
{
    data_chunk() = default;

    data_chunk(uint64_t i)
        : m_id(i)
    {
    }

    data_chunk(uint64_t i, std::vector<std::uint8_t> d)
        : m_id(i)
        , data(std::move(d))
    {
    }

    uint64_t
    get_size() const
    {
        return data.size();
    }

    uint64_t
    id() const
    {
        return m_id;
    }

    bool
    operator<(const data_chunk& other) const
    {
        return m_id < other.m_id;
    }

    uint64_t m_id = (-1);
    std::vector<std::uint8_t> data;
};

using data_chunk_ptr = std::unique_ptr<data_chunk>;

namespace utils
{

struct file_competition
{
    bool
    init(uint64_t m_file_size, uint64_t chunk_size);

    uint64_t m_file_size = 0;
    uint64_t m_chunk_size = 0;

    std::vector<bool> m_mapping;
};

struct rt_event
{
    bool
    wait();

    bool wait(std::function<bool(void)>);

    bool
    wait(std::unique_lock<std::mutex>& ul);

    void
    signal(std::unique_lock<std::mutex>& ul);

    void
    signal();

    std::unique_lock<std::mutex>
    lock();

private:
    std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_signaled = false;
};

struct load_commmnd
{
    using generic_reader = std::function<bool(const std::string& v, void* dst)>;

    static generic_reader
    to_generic_reader(const std::string&)
    {
        return read_string;
    }

    static generic_reader
    to_generic_reader(const uint64_t&)
    {
        return read_uint64;
    }

    static generic_reader
    to_generic_reader(const std::filesystem::path&)
    {
        return read_path;
    }

    std::string name;
    generic_reader loader;
    void* field_ptr = nullptr;

    static bool
    read_string(const std::string& value, void* dst)
    {
        *((std::string*)dst) = value;

        return true;
    }

    static bool
    read_uint64(const std::string& value, void* dst)
    {
        *((std::uint64_t*)dst) = std::stoull(value);

        return true;
    }

    static bool
    read_path(const std::string& value, void* dst)
    {
        *((std::filesystem::path*)dst) = value;

        return true;
    }
};

struct load_commmnds_builder
{
    template <typename T>
    load_commmnds_builder&
    add(const std::string& name, T& value)
    {
        cmds[name] = load_commmnd{
            .name = name, .loader = load_commmnd::to_generic_reader(value), .field_ptr = &value};

        return *this;
    };

    std::unordered_map<std::string, load_commmnd>
    finalize()
    {
        return std::move(cmds);
    }

    std::unordered_map<std::string, load_commmnd> cmds;
};

struct generic_kv
{
    static bool
    load(const std::filesystem::path& location,
         const std::unordered_map<std::string, load_commmnd>& cmds);
};

}  // namespace utils
}  // namespace llbridge