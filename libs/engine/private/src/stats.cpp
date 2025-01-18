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

void
statistics::stats::update()
{
    record new_state = current_state();
    auto delta = new_state - last_state;

    agg_state += delta;

    windows_records.push_back(delta);

    auto& to_drop = windows_records.front();

    agg_state -= to_drop;

    windows_records.pop_front();

    last_state = new_state;
}

statistics::record
statistics::stats::current_state()
{
    return record{.network_bytes_recv = network_bytes_recv.load(),
                  .network_bytes_send = network_bytes_send.load(),
                  .duration = ts_miliseconds()};
}

statistics::statistics(std::chrono::milliseconds delay)
    : m_running(true)
{
    m_worker = std::thread(
        [this](std::chrono::milliseconds delay)
        {
            m_state.last_state = m_state.current_state();

            while (m_running)
            {
                std::this_thread::sleep_for(delay);
                m_state.update();

                constexpr double mb = 1024.0 * 1024.0;

                const double total_network_send_mb = m_state.network_bytes_send / mb;
                const double total_network_recv_mb = m_state.network_bytes_recv / mb;
                const double total_fs_mb = m_state.fs_bytes / mb;

                auto delta_seconds = m_state.agg_state.duration * 0.001;

                const auto network_send_speed =
                    (m_state.agg_state.network_bytes_send) / (delta_seconds * mb);

                const auto network_recv_speed =
                    (m_state.agg_state.network_bytes_recv) / (delta_seconds * mb);

                constexpr auto fmt = R"(
                    delta : {:.3f} seconds
       total network send : {:.3f} MBytes
       total network recv : {:.3f} MBytes
               send speed : {:.3f} MBytes/second
               recv speed : {:.3f} MBytes/second)";

                std::cout << fmt::format(fmt, delta_seconds, total_network_send_mb,
                                         total_network_recv_mb, network_send_speed,
                                         network_recv_speed)
                          << std::endl;
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
