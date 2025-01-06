#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "engine/communication_context.h"

namespace llbridge
{
class server_impl;
class server_context;
class statistics;

class connection_session : public std::enable_shared_from_this<connection_session>
{
public:
    enum class message_result : uint32_t
    {
        next = 0,
        error,
        end
    };

    // our session holds the socket
    connection_session(std::shared_ptr<server_impl> ss,
                       boost::asio::ip::tcp::socket socket,
                       statistics* s);

    // and run was already called in our server, where we just wait for requests
    void
    run();

    communication_context&
    cc()
    {
        return m_ctx;
    }

    message_result
    on_hello(const hello_request& hr);

    message_result
    on_download_file_request(const download_file_session_request& dfs_request);

    message_result
    on_download_session_end(const download_file_session_end& rm);

    message_result
    on_chunk_request(const chunk_request& cr);

    void
    on_message_received();

private:
    void
    wait_for_message(uint64_t expected_size);

    message_header m_current_header;

    communication_context m_ctx;

    boost::asio::ip::tcp::socket m_socket;

    uint64_t m_total_read = 0;
    bool m_loading_tail = false;

    std::shared_ptr<server_impl> m_server_impl;
};

}  // namespace llbridge