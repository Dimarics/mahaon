#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/awaitable.hpp>
#include <queue>

class SerialPort
{
public:
    SerialPort(boost::asio::io_context &io_context, const std::string &dev);

    void send(std::vector<uint8_t> data);
    void shutdown();
    inline void onDataReceived(const std::function<void(const std::vector<uint8_t> &data)> &callback) {
        m_dataReceivedCallback = callback;
    }

private:
    bool m_isWriting;
    boost::asio::serial_port m_serialPort;
    std::queue<std::vector<uint8_t>> m_queue;

    boost::asio::awaitable<void> read_port();
    boost::asio::awaitable<void> do_write();

    std::function<void(const std::vector<uint8_t> &data)> m_dataReceivedCallback;
};

#endif // SERIALPORT_H
