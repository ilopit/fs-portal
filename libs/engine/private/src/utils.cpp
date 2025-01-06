#include "engine/private/utils.h"

#include <cassert>
#include <fstream>

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

void
trim_eols(std::string& str)
{
    for (auto itr = str.rbegin(); itr != str.rend() && (*itr == '\n' || *itr == '\r'); ++itr)
    {
        str.pop_back();
    }
}

bool
utils::generic_kv::load(const std::filesystem::path& location,
                        const std::unordered_map<std::string, load_commmnd>& cmds)
{
    utils::generic_kv kv;

    std::ifstream file(location);

    if (!file)
    {
        return {};
    }

    std::string tmp;
    while (std::getline(file, tmp))
    {
        if (tmp.empty())
        {
            continue;
        }

        auto pos = tmp.find_first_of("=");
        if (pos == std::string::npos)
        {
            SPDLOG_ERROR("Wrong config line {}, skipped", tmp);
            continue;
        }

        auto itr = cmds.find(tmp.substr(0, pos));

        if (itr != cmds.end())
        {
            if (!itr->second.loader(tmp.substr(pos + 1), itr->second.field_ptr))
            {
                SPDLOG_ERROR("Loader faield {}, skipped", tmp);
                return false;
            }
        }
    }
    return true;
}

}  // namespace llbridge