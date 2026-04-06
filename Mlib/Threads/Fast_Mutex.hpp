#pragma once
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <Mlib/Threads/Must_Lock_Mutex.hpp>
#include <mutex>

namespace Mlib {

#ifdef __SANITIZE_THREAD__
using FastMutex = MustLockMutex;
#else
using FastMutex = AtomicMutex;
#endif

}
