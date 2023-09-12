#include "Realtime_Threads.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef __ANDROID__
void Mlib::reserve_realtime_threads(size_t nreserved_realtime_threads) {
    // Do nothing
}
void Mlib::register_realtime_thread() {
    // Do nothing
}
void Mlib::unregister_realtime_thread() {
    // Do nothing
}
void Mlib::pin_background_thread() {
    // Do nothing
}
#elif defined(__linux__)
static std::mutex mutex_;
size_t nreserved_realtime_threads_ = SIZE_MAX;
std::unordered_map<unsigned int, pthread_t> cpu_2_thread_;

using namespace Mlib;

void Mlib::reserve_realtime_threads(size_t nreserved_realtime_threads) {
    std::scoped_lock lock{mutex_};
    if (nreserved_realtime_threads_ != SIZE_MAX) {
        THROW_OR_ABORT("Number of realtime-threads already set");
    }
    if (nreserved_realtime_threads == SIZE_MAX) {
        THROW_OR_ABORT("Invalid argument for nrealtime_threads");
    }
    if (std::thread::hardware_concurrency() < nreserved_realtime_threads) {
        THROW_OR_ABORT("Not enough CPUs available for number of real-time threads");
    }
    nreserved_realtime_threads_ = nreserved_realtime_threads;
}

static void pin_current_thread_to_cpu_range(size_t min, size_t max) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (size_t i = min; i < max; ++i) {
        CPU_SET(i, &cpuset);
    }
    if (int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset); rc != 0) {
        switch (rc) {
            case EFAULT: THROW_OR_ABORT("EFAULT: A supplied memory address was invalid.");
            case EINVAL: THROW_OR_ABORT("EINVAL: cpuset specified a CPU that was outside the set supported by the kernel");
            case ESRCH: THROW_OR_ABORT("No thread with the ID thread could be found");
            default: THROW_OR_ABORT("Unknown error calling pthread_setaffinity_np: " + std::to_string(rc));
        }
    }
}

void Mlib::register_realtime_thread() {
    std::scoped_lock lock{mutex_};
    if (nreserved_realtime_threads_ == SIZE_MAX) {
        THROW_OR_ABORT("Number of realtime-threads not set");
    }
    if (cpu_2_thread_.size() >= nreserved_realtime_threads_) {
        THROW_OR_ABORT("Not enough real-time threads reserved");
    }
    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        if (!cpu_2_thread_.contains(i)) {
            pin_current_thread_to_cpu_range(i, i + 1);
            if (!cpu_2_thread_.try_emplace(i, pthread_self()).second) {
                verbose_abort("Internal error: Could not register real-time thread (1)");
            }
            return;
        }
    }
    verbose_abort("Internal error: Could not register real-time thread (2)");
}

void Mlib::unregister_realtime_thread() {
    std::scoped_lock lock{mutex_};
    for (const auto& [c, t] : cpu_2_thread_) {
        if (t == pthread_self()) {
            cpu_2_thread_.erase(c);
            return;
        }
    }
    verbose_abort("Could not unregister real-time thread");
}

void Mlib::pin_background_thread() {
    std::scoped_lock lock{mutex_};
    if (nreserved_realtime_threads_ == SIZE_MAX) {
        THROW_OR_ABORT("Number of realtime-threads not set");
    }
    pin_current_thread_to_cpu_range(nreserved_realtime_threads_, std::thread::hardware_concurrency());
}
#else
#endif
