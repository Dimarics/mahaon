#include "mahaon/webserversession.h"
#include <iostream>

WebServerSession::WebServerSession(tcp::socket&& socket)
    : m_isWriting(false)
    , m_stream(std::move(socket)) { m_stream.text(false); }

void WebServerSession::run() {
    net::co_spawn(m_stream.get_executor(), do_accept(), net::detached);
}

// void WebServerSession::send(beast::flat_buffer data) {
//     m_queue.push(std::move(data));
//     if (!m_isWriting) {
//         m_isWriting = true;
//         net::co_spawn(m_stream.get_executor(), write_loop(), net::detached);
//     }
// }

void WebServerSession::send(std::vector<uint8_t> data) {
    m_queue.push(std::move(data));
    if (!m_isWriting) {
        m_isWriting = true;
        net::co_spawn(m_stream.get_executor(), do_write(), net::detached);
    }
}

void WebServerSession::close() {
    if (!m_stream.is_open()) return;
    m_stream.async_close(websocket::close_code::normal, net::detached);
}

void WebServerSession::on_close(const std::function<void (const std::shared_ptr<WebServerSession>&)> &callback) {
    m_onCloseCallback = callback;
}

void WebServerSession::onMessageReceived(const std::function<void (std::vector<uint8_t> &&)> &callback) {
    m_messageReceivedCallback = callback;
}

net::awaitable<void> WebServerSession::do_accept() {
    try {
        // Устанавливаем таймауты
        m_stream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

        co_await m_stream.async_accept(net::use_awaitable);

        // Только теперь запускаем цикл чтения
        net::co_spawn(m_stream.get_executor(), read_loop(), net::detached);
    } catch (const std::exception& e) {
        std::cout << "handshake failed: " << e.what() << std::endl;
        m_onCloseCallback(shared_from_this());
    }
}

net::awaitable<void> WebServerSession::read_loop() {
    beast::flat_buffer buffer;
    try {
        for (;;) {
            co_await m_stream.async_read(buffer, net::use_awaitable);
            // std::cout << "WebServerSession: msg received" << std::endl;

            const uint8_t *buf_ptr = static_cast<const uint8_t*>(buffer.data().data());
            std::vector<uint8_t> msg(buf_ptr, buf_ptr + buffer.data().size());
            m_messageReceivedCallback(std::move(msg));

            buffer.consume(buffer.size());
        }
    } catch (const std::exception& e) {
        std::cout << "disconnect: " << e.what() << std::endl;
        m_onCloseCallback(shared_from_this());
    }
}

net::awaitable<void> WebServerSession::do_write() {
    try {
        while (!m_queue.empty()) {
            std::vector<uint8_t> msg = std::move(m_queue.front());

            // std::cout << "msg: ";
            // for (auto &byte : msg) std::cout << byte;
            // std::cout << std::endl;

            co_await m_stream.async_write(net::buffer(msg), net::use_awaitable);
            // std::cout << "отправлено, в очереди: " << m_queue.size() << std::endl;
            m_queue.pop();
        }
        m_isWriting = false;
    } catch (const std::exception& e) {
        std::cout << "Запись невозможна: " << e.what() << std::endl;
        // on_close(shared_from_this());
    }
}
