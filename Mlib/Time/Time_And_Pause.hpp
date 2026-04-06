#pragma once
#include <chrono>
#include <stdexcept>

namespace Mlib {

enum class PauseStatus {
    LOADING,
    PAUSED,
    RUNNING
};

template <class TTimepoint>
class TimeAndPause {
public:
    TimeAndPause()
        : status_{PauseStatus::RUNNING}
    {}
    TimeAndPause(std::chrono::steady_clock::time_point time, PauseStatus status)
        : time_{ time }
        , status_{ status }
    {
        if (!initialized()) {
            throw std::runtime_error("TimeAndPause not initialized");
        }
    }
    ~TimeAndPause() = default;
    std::chrono::steady_clock::time_point time() const {
        if (!initialized()) {
            throw std::runtime_error("TimeAndPause not initialized");
        }
        return time_;
    }
    PauseStatus status() const {
        if (!initialized()) {
            throw std::runtime_error("TimeAndPause not initialized");
        }
        return status_;
    }
    bool initialized() const {
        return time_ != std::chrono::steady_clock::time_point();
    }
private:
    std::chrono::steady_clock::time_point time_;
    PauseStatus status_;
};

}
