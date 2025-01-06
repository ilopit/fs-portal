#pragma once

#include <engine/private/utils.h>
#include <fstream>
#include <mutex>
#include <memory>
#include <filesystem>
#include <thread>
#include <map>

namespace llbridge
{

struct write_result
{
    friend class file_cache;

public:
    write_result(data_chunk_ptr dcp);

    bool
    wait()
    {
        auto lock = m_event.lock();
        if (!m_event.wait(lock))
        {
            return false;
        }
        return m_is_done;
    }

    uint64_t
    id() const
    {
        return m_id;
    }

    void
    done()
    {
        auto lock = m_event.lock();
        m_is_done = true;
        m_event.signal(lock);
    }

    data_chunk_ptr
    dcp()
    {
        return std::move(m_dcp);
    }

private:
    uint64_t m_id = -1;
    data_chunk_ptr m_dcp;
    bool m_is_done = false;
    utils::rt_event m_event;
};

using write_result_ptr = std::shared_ptr<write_result>;

class file_recombinator
{
public:
    file_recombinator();

    ~file_recombinator();

    struct open_config
    {
        std::filesystem::path file;
        uint64_t chunk_size = 0;
        uint64_t file_size = 0;
        uint64_t batch_size = 0;
    };

    bool
    start(const open_config& rc);

    void
    stop();

    write_result_ptr
    write_chunk(data_chunk_ptr dcp);

    uint64_t
    get_offset(uint64_t chunk_id) const
    {
        return chunk_id * m_chunck_size;
    }

    uint64_t
    get_completed_chunks() const
    {
        return m_complited_chunks;
    }

    uint64_t
    get_resume_point() const
    {
        return m_resume_point;
    }

    uint64_t
    get_chunck_size(uint64_t chunk_id) const
    {
        return (chunk_id != (m_number_of_chunks - 1)) ? m_chunck_size
                                                      : (m_file_size % m_chunck_size);
    }

    data_chunk_ptr
    allocate_free_chunk(uint64_t id);

    void
    release_free_chunk(data_chunk_ptr ptr);

private:
    void
    worker_thread();

    static void
    entry_thread_main(file_recombinator& self);

    open_config m_cfg;
    uint64_t m_chunck_size = 0;
    uint64_t m_number_of_chunks = 0;
    uint64_t m_file_size = 0;
    uint64_t m_batch_size = 0;
    std::atomic<uint64_t> m_complited_chunks = 0;
    uint64_t m_resume_point = 0;

    std::thread m_worker_thread;
    std::atomic_bool m_is_running;

    std::ofstream m_file_handle;
    std::fstream m_file_lock_handle;

    utils::rt_event m_cache_event;
    utils::rt_event m_start_event;

    std::mutex m_free_lock;
    std::vector<data_chunk_ptr> m_free_chunks;

    std::map<uint64_t, write_result_ptr> m_request;
};

}  // namespace llbridge