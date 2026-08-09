#pragma once
#include <Mlib/Os/Threads/Fast_Mutex.hpp>
#include <Mlib/Os/Threads/Guarded_Object.hpp>
#include <Mlib/Os/Threads/Throwing_Lock_Guard.hpp>

namespace Mlib {

template <class T>
using ThrowingGuardedObject = SimpleGuardedObject<T, FastMutex, ThrowingLockGuard<FastMutex>>;

}
