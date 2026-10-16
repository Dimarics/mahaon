#ifndef STATEESTIMATOR_H
#define STATEESTIMATOR_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

class StateEstimator
{
public:
    StateEstimator(boost::asio::io_context &io_context);
    void shutdown();

private:
    boost::asio::steady_timer m_timer;
    void poll(const boost::system::error_code &ec);
};

#endif // STATEESTIMATOR_H
