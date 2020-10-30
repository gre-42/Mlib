#pragma once

namespace Mlib {

class AdvanceTime {
public:
    virtual void advance_time(float dt) = 0;
};

}
