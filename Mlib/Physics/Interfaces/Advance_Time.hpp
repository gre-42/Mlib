#pragma once

namespace Mlib {

class AdvanceTime {
public:
    virtual ~AdvanceTime() = default;
    virtual void advance_time(float dt) = 0;
};

}
