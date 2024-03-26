#pragma once

namespace Mlib {

class IAdvanceTime {
public:
    virtual ~IAdvanceTime() = default;
    virtual void advance_time(float dt) = 0;
};

}
