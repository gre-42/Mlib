#pragma once

namespace Mlib {

class EngineEventListener {
public:
    virtual ~EngineEventListener() = default;
    virtual void notify_off() = 0;
    virtual void notify_idle() = 0;
    virtual void notify_driving() = 0;
};

}
