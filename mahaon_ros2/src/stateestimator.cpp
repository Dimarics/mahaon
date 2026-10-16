#include "mahaon/stateestimator.h"

StateEstimator::StateEstimator(boost::asio::io_context &io_context)
    : m_timer(io_context, std::chrono::milliseconds(20))
{
    m_timer.async_wait(std::bind_front(&StateEstimator::poll, this));
}

void StateEstimator::shutdown() {
    m_timer.cancel();
}

void StateEstimator::poll(const boost::system::error_code &error_code) {
    if (error_code) return;
    m_timer.async_wait(std::bind_front(&StateEstimator::poll, this));
}
