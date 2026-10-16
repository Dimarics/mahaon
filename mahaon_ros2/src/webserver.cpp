#include "mahaon/webserver.h"
#include <iostream>

WebServer::WebServer(boost::asio::io_context &io_context, uint16_t port)
    : m_ioContext(io_context)
    , m_acceptor(nullptr)
{
    net::co_spawn(io_context, do_listen(tcp::endpoint(tcp::v4(), port)), net::detached);
}

void WebServer::shutdown() {
    boost::system::error_code error_code;
    if (m_acceptor) m_acceptor->close(error_code);
    for (auto &session : m_sessions) {
        session->close();
    }
    // std::cout << "server shutdown " << m_acceptor << std::endl;
}

void WebServer::onMessageReceived(const std::function<void (std::vector<uint8_t> &&)> &callback) {
    m_messageReceivedCallback = callback;
}

net::awaitable<void> WebServer::do_listen(tcp::endpoint endpoint) {
    auto executor = co_await net::this_coro::executor;
    auto acceptor = tcp::acceptor(executor, endpoint);
    m_acceptor = &acceptor;

    for (;;) {
        // Ждем нового TCP-клиента
        tcp::socket socket = co_await acceptor.async_accept(net::use_awaitable);

        // Запускаем сессию WebSocket для него
        // websocket::stream<beast::tcp_stream> stream(std::move(socket));
        // net::co_spawn(socket.get_executor(),
        //               do_session(std::move(stream)),
        //               net::detached);

        auto session = std::make_shared<WebServerSession>(std::move(socket));
        session->on_close([this](const std::shared_ptr<WebServerSession> &socket) {
            boost::asio::post(m_ioContext, [socket = std::move(socket), this] {
                m_sessions.erase(socket);
                std::cout << "Сессий осталось: " << m_sessions.size() << std::endl;
            });
        });
        session->onMessageReceived(m_messageReceivedCallback);
        // session->onMessageReceived([](std::vector<uint8_t>&&) { std::cout << "msg received" << std::endl; });
        m_sessions.insert(session);
        std::cout << "Сессий: " << m_sessions.size() << std::endl;
        session->run();
    }
}

// void WebServer::remove_session(const std::shared_ptr<WebServerSession> &socket) {
//     m_sessions.erase(socket);
//     std::cout << "Сессий осталось: " << m_sessions.size() << std::endl;
// }
/*
net::awaitable<void> WebServer::do_session(websocket::stream<beast::tcp_stream> stream) {
    // Настраиваем таймауты, рекомендованные для сервера
    stream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Выполняем рукопожатие (handshake)
    co_await stream.async_accept(net::use_awaitable);

    beast::flat_buffer buffer;
    for (;;) {
        // Асинхронно читаем сообщение
        co_await stream.async_read(buffer, net::use_awaitable);

        std::string msg = beast::buffers_to_string(buffer.data());
        std::cout << "Message received: " << msg << std::endl;

        buffer.consume(buffer.size()); // Очищаем буфер для следующего сообщения
    }
}
*/
