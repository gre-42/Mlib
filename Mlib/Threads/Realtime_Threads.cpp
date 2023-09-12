#include "Realtime_Threads.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef __ANDROID__
static std::mutex mutex_;
size_t nreserved_realtime_threads_ = SIZE_MAX;

void Mlib::register_realtime_thread() {
    // Do nothing
}
void Mlib::unregister_realtime_thread() {
    // Do nothing
}
#elif defined(__linux__)
static std::mutex mutex_;
size_t nreserved_realtime_threads_ = SIZE_MAX;
std::unordered_map<unsigned int, pthread_t> cpu_2_thread_;

static void pin_current_thread_to_cpu_range(unsigned int min, unsigned int max) {
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
#elif defined(_WIN32)
#include <Windows.h>
static std::mutex mutex_;
size_t nreserved_realtime_threads_ = SIZE_MAX;
std::unordered_map<unsigned int, HANDLE> cpu_2_thread_;

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

static void pin_current_thread_to_cpu_range(unsigned int min, unsigned int max) {
	DWORD_PTR affinity_mask = 0;
	for (size_t i = min; i < max; ++i) {
		affinity_mask |= (DWORD_PTR(1) << i);
	}
	if (int rc = SetThreadAffinityMask(GetCurrentThread(), affinity_mask); rc == 0) {
		THROW_OR_ABORT("Could not set thread affinity mask: " + GetLastErrorAsString());
	}
}

void Mlib::register_realtime_thread() {
	std::scoped_lock lock{ mutex_ };
	if (nreserved_realtime_threads_ == SIZE_MAX) {
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

void Mlib::reserve_realtime_threads(size_t nreserved_realtime_threads) {
	std::scoped_lock lock{ mutex_ };
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

void Mlib::pin_background_thread() {
	std::scoped_lock lock{ mutex_ };
	if (nreserved_realtime_threads_ == SIZE_MAX) {
		THROW_OR_ABORT("Number of realtime-threads not set");
	}
	pin_current_thread_to_cpu_range(nreserved_realtime_threads_, std::thread::hardware_concurrency());
}
