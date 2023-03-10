#pragma once

namespace Mlib {

class Object;

class DestructionObserver {
public:
    virtual void notify_destroyed(const Object& destroyed_object) = 0;
};

}
