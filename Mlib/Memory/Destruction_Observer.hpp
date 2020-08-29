#pragma once

namespace Mlib {

class DestructionObserver {
public:
    virtual void notify_destroyed(void* destroyed_object) = 0;
};

}
