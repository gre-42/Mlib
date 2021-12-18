#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>

namespace Mlib {

class AdvanceTimes;

class PodBots: public AdvanceTime {
public:
    PodBots(AdvanceTimes& advance_times);
    ~PodBots();
    virtual void advance_time(float dt) override;
private:
    AdvanceTimes& advance_times_;
};

}
