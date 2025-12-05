#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>

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
            THROW_OR_ABORT("TimeAndPause not initialized");
        }
    }
    ~TimeAndPause() = default;
    std::chrono::steady_clock::time_point time() const {
        if (!initialized()) {
            THROW_OR_ABORT("TimeAndPause not initialized");
        }
        return time_;
    }
    PauseStatus status() const {
        if (!initialized()) {
            THROW_OR_ABORT("TimeAndPause not initialized");
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
