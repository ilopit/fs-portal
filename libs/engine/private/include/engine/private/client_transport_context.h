#pragma once

#include <boost/asio.hpp>

namespace llbridge
{
struct client_transport_context
{
    static std::unique_ptr<client_transport_context>
    make(std::string ip, std::string port);

    client_transport_context();

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::resolver resolver;

    std::string ip;
    std::string port;
};

}  // namespace llbridge
