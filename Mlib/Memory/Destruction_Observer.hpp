#pragma once

namespace Mlib {

template <class T>
class DestructionObserver {
public:
    virtual void notify_destroyed(T destroyed_object) = 0;
};

}
