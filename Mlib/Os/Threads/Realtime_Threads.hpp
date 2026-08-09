#pragma once
#include <cstdint>

namespace Mlib {

void reserve_realtime_threads(uint32_t nreserved_realtime_threads);
void unreserve_realtime_threads();

void register_realtime_thread();
void unregister_realtime_thread();

void pin_background_thread();

class RealtimeThreadsGuard {
    RealtimeThreadsGuard(const RealtimeThreadsGuard&) = delete;
    RealtimeThreadsGuard& operator = (const RealtimeThreadsGuard&) = delete;
public:
    inline RealtimeThreadsGuard(uint32_t nreserved_realtime_threads) {
        reserve_realtime_threads(nreserved_realtime_threads);
    }
    inline ~RealtimeThreadsGuard() {
        unreserve_realtime_threads();
    }
};

class RealtimeThreadGuard {
    RealtimeThreadGuard(const RealtimeThreadGuard&) = delete;
    RealtimeThreadGuard& operator = (const RealtimeThreadGuard&) = delete;
public:
    inline RealtimeThreadGuard() {
        register_realtime_thread();
    }
    inline ~RealtimeThreadGuard() {
        unregister_realtime_thread();
    }
};

}
