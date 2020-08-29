#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>

namespace Mlib {

template <class T>
class observer_ptr {
public:
    template <class T2>
    observer_ptr(T2* ptr)
    : ptr_{ptr},
      observer_{ptr}
    {}
    observer_ptr(T* ptr, DestructionObserver* observer)
    : ptr_{ptr},
      observer_{observer}
    {}
    T* get() const {
      return ptr_;
    }
    DestructionObserver* observer() const {
      return observer_;
    }
private:
    T* ptr_;
    DestructionObserver* observer_;
};

}
