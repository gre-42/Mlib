#pragma once
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <mutex>

namespace Mlib {

#ifdef __SANITIZE_THREAD__
using FastMutex = std::mutex;
#else
using FastMutex = AtomicMutex;
#endif

}
