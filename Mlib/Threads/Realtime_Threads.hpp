#pragma once
#include <cstddef>

namespace Mlib {

void reserve_realtime_threads(size_t nreserved_realtime_threads);

void register_realtime_thread();
void unregister_realtime_thread();

void pin_background_thread();

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
