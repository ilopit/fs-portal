#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <filesystem>
#include <stdint.h>
#include <thread>
#include <fstream>
#include <map>

#include <engine/private/utils.h>

namespace llbridge
{

enum class request_state
{
    active = 0,
    available,
    empty
};

struct request_result
{
    friend class file_cache;

public:
    request_result(uint64_t i)
        : m_state(request_state::active)
        , m_id(i)
    {
    }

    bool
    read_data(data_chunk_ptr& d)
    {
        auto lock = m_event.lock();

        switch (m_state)
        {
        case request_state::active:
        {
            if (m_event.wait(lock))
            {
                d = std::move(m_chunk);
                m_state = request_state::empty;
                return true;
            }
            break;
        }
        case request_state::available:
        {
            d = std::move(m_chunk);
            m_state = request_state::empty;

            return true;
            break;
        }
        case request_state::empty:

        default:
            break;
        }

        return false;
    }

    uint64_t
    id() const
    {
        return m_id;
    }

    void
    set_data(data_chunk_ptr data);

    uint64_t m_id = -1;
    data_chunk_ptr m_chunk;
    utils::rt_event m_event;
    request_state m_state;
    bool m_send = false;
};

using read_request_ptr = std::shared_ptr<request_result>;

class file_cache
{
public:
    file_cache() = default;

    ~file_cache();

    struct open_config
    {
        std::filesystem::path file;
        uint64_t chunk_size = 0;
        uint64_t cache_size = 0;
        uint64_t read_in_advance = 0;
    };

    bool
    start(const open_config& rc);

    void
    stop();

    read_request_ptr
    read_chunk(uint64_t chunk_id);

    uint64_t
    cached_size() const
    {
        return m_cached_size;
    }

    uint64_t
    chunck_size() const
    {
        return m_chunck_size;
    }

    uint64_t
    number_of_chunks() const
    {
        return m_number_of_chunks;
    }

    uint64_t
    file_size() const
    {
        return m_file_size;
    }

    uint64_t
    get_offset(uint64_t chunk_id) const
    {
        return chunk_id * m_chunck_size;
    }

    data_chunk_ptr
    allocate_free_chunk(uint64_t from, uint64_t size);

    void
    release_free_chunk(data_chunk_ptr ptr);

private:
    void
    worker_thread();

    void
    add_to_cache(data_chunk_ptr rr);

    bool
    read_from_cache(request_result& rr);

    static void
    entry_thread_main(file_cache& self);

    void
    preload(std::uint64_t from,
            std::uint64_t to,
            const std::map<uint64_t, read_request_ptr>& to_read);

    std::uint64_t m_cached_size = {0};
    std::uint64_t m_chunck_size = {0};
    std::uint64_t m_read_in_advance = {0};
    std::uint64_t m_number_of_chunks = {0};
    std::uint64_t m_file_size = {0};

    std::ifstream m_file_handle;

    utils::rt_event m_cache_event;
    utils::rt_event m_start_event;

    std::unordered_map<uint64_t, std::unique_ptr<data_chunk>> m_cache;
    std::vector<read_request_ptr> m_requests_to_handle;
    std::queue<uint64_t> m_active_cache;

    std::thread m_worker_thread;
    std::atomic_bool m_is_running;

    std::mutex m_free_lock;
    std::vector<data_chunk_ptr> m_free_chunks;
};
}  // namespace llbridge
