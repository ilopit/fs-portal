#pragma once

#include <cstdint>
#include <thread>
#include <queue>

namespace llbridge
{

class statistics
{
public:
    statistics() = delete;

    statistics(std::chrono::milliseconds delay);

    ~statistics();

    void
    update_send(uint64_t delta)
    {
        m_state.network_bytes_send.fetch_add(delta);
    }

    void
    update_received(uint64_t delta)
    {
        m_state.network_bytes_recv.fetch_add(delta);
    }

    void
    update_fs(uint64_t delta)
    {
        m_state.fs_bytes.fetch_add(delta);
    }

private:
    struct record
    {
        uint64_t network_bytes_recv = 0;
        uint64_t network_bytes_send = 0;
        uint64_t duration = 0;

        record&
        operator+=(const record& other)
        {
            network_bytes_recv += other.network_bytes_recv;
            network_bytes_send += other.network_bytes_send;
            duration += other.duration;

            return *this;
        }

        record&
        operator-=(const record& other)
        {
            network_bytes_recv -= other.network_bytes_recv;
            network_bytes_send -= other.network_bytes_send;
            duration -= other.duration;

            return *this;
        }

        friend record
        operator-(record l, const record& r)
        {
            l -= r;
            return l;
        }
    };

    struct stats
    {
        stats()
            : windows_records(10)
        {
        }

        void
        update();

        std::atomic<uint64_t> network_bytes_recv = 0;
        std::atomic<uint64_t> network_bytes_send = 0;
        uint64_t duration = 0;
        std::atomic<uint64_t> fs_bytes = 0;

        record
        current_state();

        record last_state;
        record agg_state;

        std::deque<record> windows_records;

        uint64_t number_of_workers = 1;
    };

    stats m_state;

    std::thread m_worker;
    std::atomic<bool> m_running;
};

};  // namespace llbridge