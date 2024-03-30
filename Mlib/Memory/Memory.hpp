#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>

namespace Mlib {

template <class T, class TSender>
class observer_ptr {
public:
    template <class TDerived>
    observer_ptr(TDerived& ptr, SourceLocation loc)
        : ptr_{ ptr, loc }
        , observer_{ ptr, loc }
    {}
    observer_ptr(DanglingBaseClassPtr<T> ptr, DanglingBaseClassPtr<DestructionObserver<TSender>> observer)
        : ptr_{ ptr }
        , observer_{ observer }
    {}
    DanglingBaseClassPtr<T> get() const {
        return ptr_;
    }
    DanglingBaseClassPtr<DestructionObserver<TSender>> observer() const {
        return observer_;
    }
private:
    DanglingBaseClassPtr<T> ptr_;
    DanglingBaseClassPtr<DestructionObserver<TSender>> observer_;
};

}
