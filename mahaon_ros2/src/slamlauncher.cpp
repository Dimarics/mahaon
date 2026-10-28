#include "mahaon/slamlauncher.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/process/v2/stdio.hpp>
#include <iostream>

SlamLauncher::SlamLauncher(boost::asio::io_context &io_context)
    : m_ioContext(io_context)
    , m_process(nullptr)
{
}

void SlamLauncher::start(Mode mode) {
    if (m_process && m_process->running()) return;
    if (mode == MAPPING) {
        m_slamLaunchFile = "online_sync_launch.py";
        m_slamConfigurationFile = "slam_params_file:=/home/mes/ros2_ws/install/mahaon_ros2/share/mahaon_ros2/params/mapper_params.yaml";
        boost::asio::co_spawn(m_ioContext, runSlam(), boost::asio::detached);
    }
}

void SlamLauncher::finish() {
    // std::cout << "kill " << m_pid << std::endl;
    if (m_process && m_process->running()) {
        m_process->interrupt();
        // m_process->request_exit();
        // std::cout << m_process->running() << std::endl;
    }
}

void SlamLauncher::shutdown() {
    if (m_process && m_process->running()) {
        m_process->request_exit();
        // m_process->wait();
    }
}

boost::asio::awaitable<void> SlamLauncher::runSlam() {
    try {
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::readable_pipe out_pipe(executor);
        boost::process::v2::process process(executor, "/opt/ros/jazzy/bin/ros2",
                                            {"launch", "slam_toolbox", m_slamLaunchFile, m_slamConfigurationFile},
                                            boost::process::v2::process_stdio{ .in = nullptr, .out = out_pipe, .err = nullptr});
        m_process = &process;
        process.async_wait([this](boost::process::v2::error_code error_code, int exit_code) {
            // std::cout << "error code: " << error_code << "; exit code: " << exit_code << std::endl;
            m_process = nullptr;
        });
        char buffer[1024];
        try {
            for (;;) {
                size_t n = co_await out_pipe.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);
                if (n) std::cout.write(buffer, n);
            }
        } catch (const std::exception& e) {
            std::cout << "узел завершён: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "ros2 не выполнен" << std::endl;
    }
}
