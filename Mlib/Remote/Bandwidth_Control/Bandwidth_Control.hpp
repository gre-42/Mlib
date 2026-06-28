#pragma once
#include <Mlib/Remote/Bandwidth_Control/Bandwidth_Estimator.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <chrono>
#include <cstdint>
#include <unordered_map>

namespace Mlib {

enum class ReceiveError {
    UNKNOWN_DATAGRAM_INDEX,
    WRONG_DATAGRAM_INDEX
};

class BandwidthControl {
public:
    explicit BandwidthControl(float alpha = 0.95f, uint32_t max_payload_size = 1'000);
    ~BandwidthControl();
    bool notify_receive(DatagramIndexType datagram_index, std::istream& istr, ReceiveError& error);
    void notify_send(DatagramIndexType datagram_index, std::ostream& ostr);
    bool send_allowed(float safety_milliseconds = 30.f) const;
private:
    uint32_t max_payload_size_;
    BandwidthEstimator be_;
    std::chrono::steady_clock::time_point last_send_time_;
    std::vector<std::chrono::steady_clock::time_point> send_times_;
};

}
