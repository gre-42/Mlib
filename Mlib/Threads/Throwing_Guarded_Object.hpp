#pragma once
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/Guarded_Object.hpp>
#include <Mlib/Threads/Throwing_Lock_Guard.hpp>

namespace Mlib {

template <class T>
using ThrowingGuardedObject = SimpleGuardedObject<T, FastMutex, ThrowingLockGuard<FastMutex>>;

}
