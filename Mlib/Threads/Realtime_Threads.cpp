#include "Realtime_Threads.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef __ANDROID__
#include <sys/syscall.h>
#include <unistd.h>

static void pin_current_thread_to_cpu_range(uint32_t min, uint32_t max) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (auto i = min; i < max; ++i) {
        CPU_SET(i, &cpuset);
    }

    if (int res = sched_setaffinity(gettid(), sizeof(cpuset), &cpuset); res != 0)
    {
        switch (errno) {
            case EFAULT: THROW_OR_ABORT("EFAULT: A supplied memory address was invalid.");
            case EINVAL: THROW_OR_ABORT("EINVAL: Affinity bit mask contains no processors currently physically on the system and permitted to the process according to any restrictions that may be imposed by the \"cpuset\" mechanism");
            case ESRCH: THROW_OR_ABORT("ESRCH: The process whose ID is pid could not be found");
            default: Mlib::verbose_abort("Unknown errno: " + std::to_string(errno));
        }
    }
}
#elif defined(__linux__)
static void pin_current_thread_to_cpu_range(uint32_t min, uint32_t max) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (auto i = min; i < max; ++i) {
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
#endif

#ifdef __linux__
#include <pthread.h>
static std::mutex mutex_;
uint32_t nreserved_realtime_threads_ = UINT32_MAX;
std::unordered_map<uint32_t, pthread_t> cpu_2_thread_;

void Mlib::register_realtime_thread() {
    std::scoped_lock lock{mutex_};
    if (nreserved_realtime_threads_ == UINT32_MAX) {
        THROW_OR_ABORT("Number of realtime-threads not set");
    }
    if (cpu_2_thread_.size() >= nreserved_realtime_threads_) {
        THROW_OR_ABORT("Not enough real-time threads reserved");
    }
    for (uint32_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
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
#endif

#ifdef _WIN32
#include <Windows.h>
static std::mutex mutex_;
uint32_t nreserved_realtime_threads_ = UINT32_MAX;
std::unordered_map<uint32_t, HANDLE> cpu_2_thread_;

// From: https://stackoverflow.com/questions/1387064
static //Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

static void pin_current_thread_to_cpu_range(uint32_t min, uint32_t max) {
    DWORD_PTR affinity_mask = 0;
    for (uint32_t i = min; i < max; ++i) {
        affinity_mask |= (DWORD_PTR(1) << i);
    }
    if (DWORD_PTR rc = SetThreadAffinityMask(GetCurrentThread(), affinity_mask); rc == 0) {
        THROW_OR_ABORT("Could not set thread affinity mask: " + GetLastErrorAsString());
    }
}

void Mlib::register_realtime_thread() {
    std::scoped_lock lock{ mutex_ };
    if (nreserved_realtime_threads_ == UINT32_MAX) {
        THROW_OR_ABORT("Number of realtime-threads not set");
    }
    if (cpu_2_thread_.size() >= nreserved_realtime_threads_) {
        THROW_OR_ABORT("Not enough real-time threads reserved");
    }
    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        if (!cpu_2_thread_.contains(i)) {
            pin_current_thread_to_cpu_range(i, i + 1);
            if (!cpu_2_thread_.try_emplace(i, GetCurrentThread()).second) {
                verbose_abort("Internal error: Could not register real-time thread (1)");
            }
            return;
        }
    }
    verbose_abort("Internal error: Could not register real-time thread (2)");
}

void Mlib::unregister_realtime_thread() {
    std::scoped_lock lock{ mutex_ };
    for (const auto& [c, t] : cpu_2_thread_) {
        if (t == GetCurrentThread()) {
            cpu_2_thread_.erase(c);
            return;
        }
    }
    verbose_abort("Could not unregister real-time thread");
}

#endif

void Mlib::reserve_realtime_threads(uint32_t nreserved_realtime_threads) {
    std::scoped_lock lock{ mutex_ };
    if (nreserved_realtime_threads_ != UINT32_MAX) {
        THROW_OR_ABORT("Number of realtime-threads already set");
    }
    if (nreserved_realtime_threads == UINT32_MAX) {
        THROW_OR_ABORT("Invalid argument for nrealtime_threads");
    }
    if (std::thread::hardware_concurrency() < nreserved_realtime_threads) {
        THROW_OR_ABORT("Not enough CPUs available for number of real-time threads");
    }
    nreserved_realtime_threads_ = nreserved_realtime_threads;
}

void Mlib::unreserve_realtime_threads() {
    std::scoped_lock lock{ mutex_ };
    if (nreserved_realtime_threads_ == UINT32_MAX) {
        verbose_abort("Number of realtime-threads not set");
    }
    if (!cpu_2_thread_.empty()) {
        verbose_abort("CPU-to-thread mapping not empty");
    }
    nreserved_realtime_threads_ = UINT32_MAX;
}

void Mlib::pin_background_thread() {
    std::scoped_lock lock{ mutex_ };
    if (nreserved_realtime_threads_ == UINT32_MAX) {
        THROW_OR_ABORT("Number of realtime-threads not set");
    }
    pin_current_thread_to_cpu_range(nreserved_realtime_threads_, std::thread::hardware_concurrency());
}
