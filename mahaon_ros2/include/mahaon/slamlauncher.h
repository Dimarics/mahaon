#ifndef SLAMLAUNCHER_H
#define SLAMLAUNCHER_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/process/v2/process.hpp>

class SlamLauncher
{
public:
    enum Mode {
        MAPPING,
        LOCALIZATION
    };
    SlamLauncher(boost::asio::io_context &io_context);
    void start(Mode mode);
    void finish();
    void shutdown();
    inline bool running() const { return (m_process && m_process->running()); }

private:
    std::string m_slamLaunchFile;
    std::string m_slamConfigurationFile;
    boost::asio::io_context &m_ioContext;
    boost::process::v2::process *m_process;

    boost::asio::awaitable<void> runSlam();
};

#endif // SLAMLAUNCHER_H
