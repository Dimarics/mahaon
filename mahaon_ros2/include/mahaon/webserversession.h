#ifndef WEBSERVERSESSION_H
#define WEBSERVERSESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <queue>

class Server;

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebServerSession : public std::enable_shared_from_this<WebServerSession>
{
public:
    WebServerSession(tcp::socket&& socket);
    void run();
    // void send(beast::flat_buffer data);
    void send(std::vector<uint8_t> data);
    void close();
    void on_close(const std::function<void(const std::shared_ptr<WebServerSession>&)> &callback);
    void onMessageReceived(const std::function<void(std::vector<uint8_t>&&)> &callback);

private:
    bool m_isWriting;
    net::any_io_executor m_executor;
    websocket::stream<beast::tcp_stream> m_stream;
    std::queue<std::vector<uint8_t>> m_queue;
    std::function<void(const std::shared_ptr<WebServerSession>&)> m_onCloseCallback;
    std::function<void(std::vector<uint8_t>&&)> m_messageReceivedCallback;

    net::awaitable<void> do_accept();
    net::awaitable<void> read_loop();
    net::awaitable<void> do_write();
};

#endif // WEBSERVERSESSION_H
