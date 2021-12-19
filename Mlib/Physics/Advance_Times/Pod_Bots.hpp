#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <chrono>

namespace Mlib {

class AdvanceTimes;
class Players;

class PodBots: public AdvanceTime {
public:
    PodBots(AdvanceTimes& advance_times, Players& players);
    ~PodBots();
    virtual void advance_time(float dt) override;
private:
    AdvanceTimes& advance_times_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
