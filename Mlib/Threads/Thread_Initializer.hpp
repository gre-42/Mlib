#pragma once
#include <Mlib/Threads/Malloc_Map.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <optional>
#include <string>

namespace Mlib {

enum class ThreadAffinity;

class ThreadInitializer {
public:
    ThreadInitializer(
        const std::string& name,
        ThreadAffinity affinity);
    ~ThreadInitializer();
private:
    std::optional<RealtimeThreadGuard> rtg_;
    #ifdef MALLOC_WRAPPING_ENABLED
    MallocGuard malloc_guard_;
    #endif
};

}
