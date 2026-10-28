#ifndef IMU_H
#define IMU_H

#include "bno085.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

class IMU
{
public:
    IMU(boost::asio::io_context &io_context);
    void shutdown();
    inline void onYawChanged(const std::function<void (double)> &callback) {
        m_yawChangedCallback = callback;
    }

private:
    bool m_connected;
    TwoWire m_wire;
    Adafruit_BNO08x m_bno085;
    sh2_SensorValue_t m_sensorValue;
    boost::asio::steady_timer m_timer;
    void poll(const boost::system::error_code &ec);

    std::function<void (double)> m_yawChangedCallback;
};

#endif // IMU_H
