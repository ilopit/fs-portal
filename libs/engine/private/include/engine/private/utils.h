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

}  // namespace utils
}  // namespace llbridge