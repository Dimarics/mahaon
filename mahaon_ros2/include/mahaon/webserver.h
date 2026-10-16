#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "mahaon/webserversession.h"
#include "flatbuf_datastream.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <unordered_set>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebServer
{
public:
    WebServer(boost::asio::io_context &io_context, uint16_t port);
    void shutdown();

    inline void send(beast::flat_buffer data) {
        const uint8_t *ptr = static_cast<const uint8_t*>(data.data().data());
        size_t size = data.data().size();
        std::vector<uint8_t> buffer(ptr, ptr + size);
        for (auto &session : m_sessions) session->send(buffer);
    }
    inline void send(const std::vector<uint8_t> buffer) {
        for (auto &session : m_sessions) session->send(buffer);
    }
    template <typename... Args>
    void send(const Args&... args) {
        beast::flat_buffer buffer;
        FlatBufWriter writer(buffer);
        (writer << ... << args);
        send(buffer);
    }
    void onMessageReceived(const std::function<void(std::vector<uint8_t>&&)> &callback);

private:
    boost::asio::io_context &m_ioContext;
    tcp::acceptor *m_acceptor;
    std::unordered_set<std::shared_ptr<WebServerSession>> m_sessions;

    net::awaitable<void> do_listen(tcp::endpoint endpoint);
    // void remove_session(const std::shared_ptr<WebServerSession>& socket);
    // net::awaitable<void> do_session(websocket::stream<beast::tcp_stream> stream);
    std::function<void(std::vector<uint8_t>&&)> m_messageReceivedCallback;
};

#endif // WEBSERVER_H
