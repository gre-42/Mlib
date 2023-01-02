#pragma once

namespace Mlib {

class Object;

class DestructionObserver {
public:
    virtual void notify_destroyed(Object& destroyed_object) = 0;
};

}
