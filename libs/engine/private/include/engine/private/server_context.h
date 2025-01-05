#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <unordered_map>
#include <filesystem>

#include "engine/private/file_read_cache.h"

#include <engine/server.h>
#include <engine/files_list.h>

namespace llbridge
{
struct server_transport_context
{
    static std::unique_ptr<server_transport_context>
    make(std::string port);

    void
    start();

    void
    stop();

    server_transport_context(std::uint16_t port);

    boost::asio::io_context m_io_context;
    boost::asio::io_context::work m_work;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::thread m_worker;
};

}  // namespace llbridge