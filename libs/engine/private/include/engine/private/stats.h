#pragma once

#include <cstdint>
#include <thread>

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
        m_state.network_bytes_received.fetch_add(delta);
    }

    void
    update_fs(uint64_t delta)
    {
        m_state.fs_bytes.fetch_add(delta);
    }

private:
    struct stats
    {
        std::atomic<uint64_t> network_bytes_received = 0;
        std::atomic<uint64_t> network_bytes_send = 0;
        std::atomic<uint64_t> fs_bytes = 0;
        uint64_t last_network_bytes_recv = 0;
        uint64_t last_network_bytes_send = 0;
        uint64_t number_of_workers = 1;
    };

    stats m_state;

    std::thread m_worker;
    std::atomic<bool> m_running;
};

};  // namespace llbridge