#include <engine/private/stats.h>
#include <iostream>
#include <spdlog/spdlog.h>

namespace llbridge
{

namespace
{

uint64_t
ts_miliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}
}  // namespace

statistics::statistics(std::chrono::milliseconds delay)
    : m_running(true)
{
    m_worker = std::thread(
        [this](std::chrono::milliseconds delay)
        {
            uint64_t time_stamps = ts_miliseconds();

            while (m_running)
            {
                std::this_thread::sleep_for(delay);

                const auto network_send_bytes = m_state.network_bytes_send.load();
                const auto network_recv_bytes = m_state.network_bytes_received.load();
                const auto fs = m_state.fs_bytes.load();

                constexpr double mb = 1024.0 * 1024.0;

                const double total_network_send_mb = network_send_bytes / mb;
                const double total_network_receive_mb = network_recv_bytes / mb;
                const double total_fs_mb = m_state.fs_bytes / mb;

                auto delta_seconds = (ts_miliseconds() - time_stamps) * 0.001;

                const auto network_send_speed =
                    (network_send_bytes - m_state.last_network_bytes_send) / (delta_seconds * mb);

                const auto network_recv_speed =
                    (network_recv_bytes - m_state.last_network_bytes_recv) / (delta_seconds * mb);

                constexpr auto fmt = R"(
                    delta : {:.3f} seconds
       total network send : {:.3f} MBytes
       total network recv : {:.3f} MBytes
               send speed : {:.3f} MBytes/second
               recv speed : {:.3f} MBytes/second)";

                std::cout << fmt::format(fmt, delta_seconds, total_network_send_mb,
                                         total_network_receive_mb, network_send_speed,
                                         network_recv_speed)
                          << std::endl;

                time_stamps = ts_miliseconds();
                m_state.last_network_bytes_send = network_send_bytes;
                m_state.last_network_bytes_recv = network_recv_bytes;
            }
        },
        delay);
}

statistics::~statistics()
{
    m_running = false;

    if (m_worker.joinable())
    {
        m_worker.join();
    }
}

}  // namespace llbridge
