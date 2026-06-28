#include "Bandwidth_Control.hpp"
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <limits>

using namespace Mlib;

BandwidthControl::BandwidthControl(float alpha, uint32_t max_payload_size)
    : max_payload_size_{max_payload_size}
    , be_{alpha}
    , send_times_(std::numeric_limits<DatagramIndexType>::max() + 1)
{}

BandwidthControl::~BandwidthControl() = default;

bool BandwidthControl::notify_receive(
    DatagramIndexType datagram_index,
    std::istream& istr,
    ReceiveError& error)
{
    auto& send_time = send_times_.at(datagram_index);
    if (send_time == std::chrono::steady_clock::time_point()) {
        error = ReceiveError::UNKNOWN_DATAGRAM_INDEX;
        return false;
    }
    auto duration = (std::chrono::steady_clock::now() - send_time);
    if (duration.count() < 0) {
        error = ReceiveError::WRONG_DATAGRAM_INDEX;
        return false;
    }
    send_time = {};
    return true;
}

void BandwidthControl::notify_send(DatagramIndexType datagram_index, std::ostream& ostr) {
    last_send_time_ = std::chrono::steady_clock::now();
}

bool BandwidthControl::send_allowed(float safety_milliseconds) const {
    if (last_send_time_ == std::chrono::steady_clock::time_point()) {
        return true;
    }
    auto duration = std::chrono::steady_clock::now() - last_send_time_;
    return duration > std::chrono::duration<float>(safety_milliseconds / 1e3f + (integral_to_float<float>(max_payload_size_) + be_.header_size()) / be_.bandwidth());
}
