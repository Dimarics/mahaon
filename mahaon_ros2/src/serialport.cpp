#include "mahaon/serialport.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>

SerialPort::SerialPort(boost::asio::io_context &io_context, const std::string &dev)
    : m_isWriting(false), m_serialPort(io_context, dev)
{
    using asio_serial = boost::asio::serial_port;
    m_serialPort.set_option(asio_serial::baud_rate(9600));
    m_serialPort.set_option(asio_serial::character_size(8));
    m_serialPort.set_option(asio_serial::parity(asio_serial::parity::none));
    m_serialPort.set_option(asio_serial::stop_bits(asio_serial::stop_bits::one));
    m_serialPort.set_option(asio_serial::flow_control(asio_serial::flow_control::none));
    boost::asio::co_spawn(io_context, read_port(), boost::asio::detached);
}

void SerialPort::send(std::vector<uint8_t> data) {
    m_queue.push(std::move(data));
    // std::cout << "в очереди: " << m_queue.size() << std::endl;
    if (!m_isWriting) {
        m_isWriting = true;
        boost::asio::co_spawn(m_serialPort.get_executor(), do_write(), boost::asio::detached);
    }
}

void SerialPort::shutdown() {
    m_serialPort.close();

    // для потокобезопасности
    // boost::asio::post(m_serialPort.get_executor(), [this] {
    //     m_serialPort.close();
    // });
}

boost::asio::awaitable<void> SerialPort::read_port() {
    uint8_t buffer[1024];
    for (;;) {
        std::size_t n = co_await m_serialPort.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);
        // std::cout << "Прочитано " << n << " байт" << std::endl;
        std::vector<uint8_t> data(std::begin(buffer), std::begin(buffer) + n);
        m_dataReceivedCallback(data);
    }
}

boost::asio::awaitable<void> SerialPort::do_write() {
    try {
        while (!m_queue.empty()) {
            std::vector<uint8_t> msg = std::move(m_queue.front());
            co_await m_serialPort.async_write_some(boost::asio::buffer(msg), boost::asio::use_awaitable);
            m_queue.pop();
        }
        m_isWriting = false;
    } catch (const std::exception& e) {
        std::cout << "Запись невозможна: " << e.what() << std::endl;
    }
}
