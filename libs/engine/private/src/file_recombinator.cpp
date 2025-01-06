#include <engine/private/file_recombinator.h>
#include <engine/private/psx_file.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <cstdio>

namespace llbridge
{

file_recombinator::file_recombinator()
{
}

file_recombinator::~file_recombinator()
{
    stop();
}

bool
file_recombinator::start(const open_config& rc)
{
    if (m_is_running)
    {
        return false;
    }
    m_cfg = rc;

    if (!rc.file.is_absolute())
    {
        return false;
    }

    std::error_code ec;

    if (!std::filesystem::exists(rc.file.parent_path(), ec) &&
        !std::filesystem::create_directories(rc.file.parent_path(), ec))
    {
        SPDLOG_ERROR("Unable to create a directory {}", ec.message());

        return false;
    }

    m_file_handle.open(rc.file, std::ios_base::binary | std::fstream::out | std::fstream::app);

    if (!m_file_handle.is_open())
    {
        return false;
    }

    auto lock_file = rc.file.parent_path() / (rc.file.filename().generic_string() + ".tmp");
    uint64_t offset = 0;

    m_file_lock_handle.open(lock_file, std::fstream::app | std::fstream::in | std::fstream::out);
    if (!m_file_lock_handle.is_open())
    {
        return false;
    }

    m_file_lock_handle >> offset;

    auto current_file_size = (uint64_t)m_file_handle.seekp(0, std::ios_base::end).tellp();
    m_file_handle.seekp(0, std::ios_base::beg);

    SPDLOG_INFO("Last recorded offset {}, file_size {}", offset, current_file_size);
    if (current_file_size < offset)
    {
        SPDLOG_ERROR("Unrecovarable");
        return false;
    }

    m_file_size = rc.file_size;

    // m_complited_chunks = offset / rc.chunk_size;

    m_chunck_size = rc.chunk_size;
    m_batch_size = rc.batch_size;
    m_number_of_chunks = m_number_of_chunks =
        m_file_size / m_chunck_size + ((m_file_size % m_chunck_size) ? 1 : 0);

    m_is_running = true;
    m_worker_thread = std::thread(file_recombinator::entry_thread_main, std::ref(*this));
    m_start_event.wait();

    SPDLOG_INFO(
        "file_recombinator::start\n  file = {}\n  chunck_size = {}\n  batch_size= {}\n  "
        "file_size = {}\n  number_of_chunks = {}",
        rc.file.generic_string(), m_chunck_size, m_batch_size, m_file_size, m_number_of_chunks);

    m_file_handle.clear();
    m_file_handle.close();
    return true;
}

void
file_recombinator::stop()
{
    if (m_is_running)
    {
        m_is_running = false;
        m_cache_event.signal();

        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }
    }
}

write_result_ptr
file_recombinator::write_chunk(data_chunk_ptr dcp)
{
    auto ptr = std::make_shared<write_result>(std::move(dcp));

    auto lock = m_cache_event.lock();

    auto& r = m_request[ptr->id()];

    if (r)
    {
        SPDLOG_ERROR("Overriding!");
    }

    r = ptr;

    m_cache_event.signal(lock);

    return ptr;
}

data_chunk_ptr
file_recombinator::allocate_free_chunk(uint64_t id)
{
    data_chunk_ptr r = nullptr;

    {
        std::lock_guard lg(m_free_lock);

        if (!m_free_chunks.empty())
        {
            r = std::move(m_free_chunks.back());
            m_free_chunks.pop_back();
        }
    }

    if (!r)
    {
        r = std::make_unique<data_chunk>(id);
    }
    r->m_id = id;

    // r->data.resize(get_chunck_size(id));

    return r;
}

void
file_recombinator::release_free_chunk(data_chunk_ptr ptr)
{
    if (!ptr)
    {
        return;
    }

    std::lock_guard lg(m_free_lock);

    m_free_chunks.emplace_back(std::move(ptr));
}

void
file_recombinator::worker_thread()
{
    SPDLOG_WARN("Starting file recombinator!");

    m_start_event.signal();

    std::map<uint64_t, write_result_ptr> to_handle;
    psx_file fh(m_cfg.file);

    if (!fh.is_open())
    {
        goto clean_up;
    }

    while (m_is_running)
    {
        {
            auto lock = m_cache_event.lock();
            if (m_request.size() < m_batch_size &&
                m_request.size() != (m_number_of_chunks - m_complited_chunks))
            {
                m_cache_event.wait(lock);
            }
            m_request.swap(to_handle);
        }

        for (auto& item : to_handle)
        {
            auto dcp = item.second->dcp();

            auto offset = get_offset(dcp->id());

            SPDLOG_INFO("Writing id {} , size {}, at {}", dcp->id(), dcp->get_size(), offset);

            if (!fh.seek_to(offset, SEEK_SET))
            {
                SPDLOG_ERROR("fseek failed");
                goto clean_up;
            }

            auto wr_size = fh.write(dcp->data);
            if (wr_size != dcp->get_size())
            {
                SPDLOG_ERROR("fwrite failed {}", dcp->get_size());
                goto clean_up;
            }
            fh.flush();

            {
                std::lock_guard lg(m_free_lock);

                m_free_chunks.push_back(std::move(dcp));
            }

            item.second->done();
            m_complited_chunks++;

            m_file_lock_handle << offset;
            m_file_lock_handle.flush();
        }

        to_handle.clear();
    }

clean_up:

    for (auto& h : to_handle)
    {
        h.second->done();
    }
    for (auto& h : m_request)
    {
        h.second->done();
    }

    SPDLOG_WARN("End file recombinator!");
}

void
file_recombinator::entry_thread_main(file_recombinator& self)
{
    self.worker_thread();
}

write_result::write_result(data_chunk_ptr dcp)
    : m_id(dcp->id())
    , m_dcp(std::move(dcp))
{
}

}  // namespace llbridge
