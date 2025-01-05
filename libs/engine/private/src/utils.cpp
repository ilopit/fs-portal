#include "engine/private/utils.h"

#include <cassert>

namespace llbridge

{

bool
utils::file_competition::init(uint64_t file_size, uint64_t chunk_size)
{
    m_file_size = file_size;
    m_chunk_size = chunk_size;

    uint64_t chunks = m_file_size / m_chunk_size + (m_file_size % m_chunk_size ? 1 : 0);

    m_mapping.resize(chunks, false);

    return 0;
}

bool
utils::rt_event::wait()
{
    std::unique_lock<std::mutex> ul(m_lock);
    wait(ul);

    return true;
}

bool
utils::rt_event::wait(std::function<bool(void)>)
{
    return true;
}

bool
utils::rt_event::wait(std::unique_lock<std::mutex>& ul)
{
    assert(ul.mutex() == &m_lock && "Miss match");

    m_cv.wait(ul, [this]() { return m_signaled; });

    return true;
}

void
utils::rt_event::signal()
{
    std::unique_lock<std::mutex> ul(m_lock);

    signal(ul);
}

void
utils::rt_event::signal(std::unique_lock<std::mutex>& ul)
{
    assert(ul.mutex() == &m_lock && "Miss match");
    assert(ul.owns_lock() && "Miss match");

    m_signaled = true;
    m_cv.notify_all();
}

std::unique_lock<std::mutex>
utils::rt_event::lock()
{
    return std::unique_lock<std::mutex>(m_lock);
}

}  // namespace llbridge