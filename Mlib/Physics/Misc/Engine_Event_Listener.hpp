#pragma once

namespace Mlib {

class EngineEventListener {
public:
    virtual ~EngineEventListener() = default;
    virtual void notify_off() = 0;
    virtual void notify_idle(float w) = 0;
    virtual void notify_driving(float w) = 0;
};

}
