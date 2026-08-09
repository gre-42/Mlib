#pragma once
#include <Mlib/Os/Threads/Atomic_Mutex.hpp>
#include <Mlib/Os/Threads/Must_Lock_Mutex.hpp>
#include <mutex>

namespace Mlib {

#ifdef __SANITIZE_THREAD__
using FastMutex = MustLockMutex;
#else
using FastMutex = AtomicMutex;
#endif

}
