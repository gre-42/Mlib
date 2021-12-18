#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>

namespace Mlib {

class PodBots: public AdvanceTime {
    virtual void advance_time(float dt) override;

};

}
