#include <engine/private/file_read_cache.h>

#include <spdlog/spdlog.h>
#include <map>

namespace llbridge
{
file_cache::~file_cache()
{
    stop();
}

bool
file_cache::start(const open_config& rc)
{
    if (m_is_running)
    {
        return false;
    }

    m_chunck_size = rc.chunk_size;
    m_cached_size = rc.cache_size;
    m_read_in_advance = rc.read_in_advance;

    m_file_handle.open(rc.file, std::ios_base::binary | std::fstream::in | std::ifstream::ate);

    if (!m_file_handle.is_open())
    {
        return false;
    }

    m_file_size = m_file_handle.tellg();

    m_number_of_chunks = m_file_size / m_chunck_size + ((m_file_size % m_chunck_size) ? 1 : 0);
    m_file_handle.seekg(0, std::ios_base::beg);
    m_is_running = true;

    SPDLOG_INFO(
        "file_cache::start\n  file = {} \n  chunck_size = {} \n  cached_size= {} \n  "
        "m_read_in_advance= {} "
        "\n  file_size = "
        "{} \n  number_of_chunks = {}",
        rc.file.generic_string(), m_chunck_size, m_cached_size, m_read_in_advance, m_file_size,
        m_number_of_chunks);

    m_worker_thread = std::thread(file_cache::entry_thread_main, std::ref(*this));

    m_start_event.wait();

    return true;
}

void
file_cache::stop()
{
    m_is_running = false;

    if (m_worker_thread.joinable())
    {
        m_worker_thread.join();
    }
}

read_request_ptr
file_cache::read_chunk(uint64_t chunk_id)
{
    auto ptr = std::make_shared<request_result>(chunk_id);

    {
        auto lock = m_cache_event.lock();
        if (read_from_cache(*ptr))
        {
            return ptr;
        }
    }

    {
        auto lock = m_cache_event.lock();
        m_requests_to_handle.push_back(ptr);
        m_cache_event.signal(lock);
    }

    return ptr;
}

data_chunk_ptr
file_cache::allocate_free_chunk(uint64_t id, uint64_t size)
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

    r->data.resize(size);

    return r;
}

void
file_cache::release_free_chunk(data_chunk_ptr ptr)
{
    if (!ptr)
    {
        return;
    }

    std::lock_guard lg(m_free_lock);

    m_free_chunks.emplace_back(std::move(ptr));
}

void
file_cache::worker_thread()
{
    m_start_event.signal();

    while (m_is_running)
    {
        std::vector<read_request_ptr> to_handle;

        {
            auto lock = m_cache_event.lock();
            if (m_requests_to_handle.empty())
            {
                m_cache_event.wait(lock);
            }

            m_requests_to_handle.swap(to_handle);
        }

        if (to_handle.empty())
        {
            continue;
        }

        std::map<uint64_t, read_request_ptr> to_read;
        // cache hit
        {
            auto lock = m_cache_event.lock();

            for (const auto& h : to_handle)
            {
                auto& c = *h;
                if (!read_from_cache(c))
                {
                    to_read[c.id()] = h;
                }
            }
        }

        {
            uint64_t last_preload = -1;
            for (auto& item_to_read : to_read)
            {
                if (item_to_read.second->m_send)
                {
                    continue;
                }

                auto id = item_to_read.first;

                if (last_preload == -1 || last_preload < id)
                {
                    last_preload = std::min(id + m_read_in_advance, (m_number_of_chunks - 1));

                    preload(id, last_preload, to_read);
                }
                {
                    auto lock = m_cache_event.lock();
                    read_from_cache(*item_to_read.second);
                }
            }
        }
    }

    SPDLOG_ERROR("Stopped");
}

void
file_cache::add_to_cache(data_chunk_ptr dcp)
{
    auto ul = m_cache_event.lock();

    m_active_cache.push(dcp->id());
    m_cache[dcp->id()] = std::move(dcp);
}

bool
file_cache::read_from_cache(request_result& rr)
{
    auto itr = m_cache.find(rr.id());

    if (itr == m_cache.end())
    {
        return false;
    }

    rr.set_data(std::move(itr->second));

    m_cache.erase(itr);

    return true;
}

void
file_cache::entry_thread_main(file_cache& self)
{
    self.worker_thread();
}

void
file_cache::preload(std::uint64_t from,
                    std::uint64_t to,
                    const std::map<uint64_t, read_request_ptr>& to_read)
{
    auto offset = get_offset(from);
    if (m_file_handle.tellg() != offset)
    {
        m_file_handle.seekg(offset, std::ios_base::beg);
    }

    for (; from <= to; ++from)
    {
        {
            auto ul = m_cache_event.lock();
            if (m_cache.count(from))
            {
                continue;
            }
        }

        auto size_to_read = std::min(m_file_size - offset, m_chunck_size);

        auto dc = allocate_free_chunk(from, size_to_read);

        m_file_handle.read((char*)dc->data.data(), size_to_read);

        auto readed = m_file_handle.gcount();

        if (size_to_read != readed)
        {
            SPDLOG_ERROR("Miss readed data expected - {}, readed - {}", size_to_read, readed);
        }

        offset += readed;

        auto itr = to_read.find(dc->id());
        if (itr == to_read.end())
        {
            add_to_cache(std::move(dc));
        }
        else
        {
            auto& rqst = itr->second;

            if (!rqst->m_send)
            {
                rqst->set_data(std::move(dc));
            }
        }
    }
}

void
request_result::set_data(data_chunk_ptr data)
{
    m_send = true;
    auto lock = m_event.lock();
    m_chunk = std::move(data);
    m_state = request_state::available;

    m_event.signal(lock);
}

}  // namespace llbridge