#ifndef IMU_H
#define IMU_H

#include <thread>
#include <boost/asio/io_context.hpp>

class IMU
{
public:
    IMU(boost::asio::io_context &io_context);
    void shutdown();
    inline void onRawChanged(const std::function<void (double, double, double, double, double, double)> &callback) {
        m_rawChangedCallback = callback;
    }
    inline void onRotationChanged(const std::function<void (double, double, double)> &callback) {
        m_rotationChangedCallback = callback;
    }

private:
    std::thread m_thread;
    std::atomic_bool m_running;

    std::function<void (double, double, double, double, double, double)> m_rawChangedCallback;
    std::function<void (double, double, double)> m_rotationChangedCallback;
};

#endif // IMU_H
