#pragma once

#include <boost/asio.hpp>

#include <memory>
#include <string>

namespace llbridge
{

class connection
{
    friend class client_transport_context;

public:
    connection(boost::asio::ip::tcp::socket s)
        : m_socket(std::move(s))
    {
    }

    ~connection()
    {
        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close();
    }

    boost::asio::ip::tcp::socket&
    socket()
    {
        return m_socket;
    }

private:
    boost::asio::ip::tcp::socket m_socket;
};

class client_transport_context
{
public:
    static std::unique_ptr<client_transport_context>
    make(std::string ip, std::string port);

    client_transport_context();

    client_transport_context(std::string ip, std::string port);

    std::unique_ptr<connection>
    connect();

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::resolver resolver;

    std::string ip;
    std::string port;
};

}  // namespace llbridge
