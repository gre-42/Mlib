#ifdef MALLOC_WRAPPING_ENABLED

#include "Malloc_Map.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

using namespace Mlib;

static THREAD_LOCAL(const char*) current_;
static std::unordered_map<const char*, size_t> allocated_;
static FastMutex allocated_mutex_;

MallocGuard::MallocGuard(const char* name)
    : parent_{current_}
{
    current_ = name;
}

MallocGuard::~MallocGuard() {
    current_ = parent_;
}

extern "C" void* __real_malloc(size_t size);

extern "C" void* __wrap_malloc(size_t size) {
    if (current_ != nullptr) {
        std::scoped_lock lock{allocated_mutex_};
        allocated_[current_] += size;
    }
    return __real_malloc(size);
}

extern "C" {
    // operator new(size_t)
    void* __real__Znwm(std::size_t size);
    void* __wrap__Znwm(std::size_t size) {
        if (current_ != nullptr) {
            auto c = current_;
            MallocGuard malloc_guard{nullptr};
            std::scoped_lock lock{allocated_mutex_};
            allocated_[c] += size;
        }
        return __real__Znwm(size);
    }
    // operator new[](size_t)
    void* __real__Znam(std::size_t size);
    void* __wrap__Znam(std::size_t size) {
        if (current_ != nullptr) {
            auto c = current_;
            MallocGuard malloc_guard{nullptr};
            std::scoped_lock lock{allocated_mutex_};
            allocated_[c] += size;
        }
        return __real__Znam(size);
    }
    // operator new(size_t, std::align_val_t)
    void* __real__ZnwmSt11align_val_t(size_t size, std::align_val_t align);
    void* __wrap__ZnwmSt11align_val_t(size_t size, std::align_val_t align) {
        if (current_ != nullptr) {
            auto c = current_;
            MallocGuard malloc_guard{nullptr};
            std::scoped_lock lock{allocated_mutex_};
            allocated_[c] += size;
        }
        return __real__ZnwmSt11align_val_t(size, align);
    }
}

void print_size(std::ostream& ostr, size_t s) {
    if (s >= 1000'000'000) {
        ostr << (double(s) / 1e9) << "GB";
    } else if (s >= 1000'000) {
    ostr << (double(s) / 1e6) << "MB";
    } else if (s >= 1000) {
        ostr << (double(s) / 1e3) << "kB";
    } else {
        ostr << s << 'B';
    }
}

void Mlib::print_allocated() {
    linfo() << "Allocated begin";
    std::scoped_lock lock{allocated_mutex_};
    for (const auto& [k, v] : allocated_) {
        auto l = linfo();
        l << ((k == nullptr) ? "?" : k) << ": ";
        print_size(l, v);
    }
    linfo() << "Allocated end";
}

#endif
