#include <engine/private/file_recombinator.h>

#include <spdlog/spdlog.h>

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

    std::error_code ec;
    std::filesystem::create_directories(rc.file.parent_path(), ec);

    m_file_handle.open(rc.file, std::ios_base::binary | std::fstream::out);

    if (!m_file_handle.is_open())
    {
        return false;
    }
    m_file_size = rc.file_size;

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
    m_start_event.signal();

    while (m_is_running)
    {
        std::map<uint64_t, write_result_ptr> to_handle;

        auto lock = m_cache_event.lock();

        if (m_request.size() < m_batch_size &&
            m_request.size() != (m_number_of_chunks - m_complited_chunks))
        {
            m_cache_event.wait(lock);
        }

        m_request.swap(to_handle);

        for (auto& item : to_handle)
        {
            auto dcp = item.second->dcp();

            m_file_handle.seekp(get_offset(dcp->id()));

            m_file_handle.write((char*)dcp->data.data(), dcp->get_size());

            {
                std::lock_guard lg(m_free_lock);

                m_free_chunks.push_back(std::move(dcp));
            }

            item.second->done();
            m_complited_chunks++;
        }
    }

    m_file_handle.close();
}

void
file_recombinator::entry_thread_main(file_recombinator& self)
{
    self.worker_thread();
}

}  // namespace llbridge
