#pragma once
#include <chrono>
#include <map>
#include <thread>
#include <vector>

namespace Mlib {

template <class TSize>
class Svg;

struct TimeEvent {
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
    const char* message = nullptr;
    size_t stack_size = SIZE_MAX;
};

class TimeGuard {
public:
    TimeGuard(const char* message);
    ~TimeGuard();
    static void set_max_log_length(size_t len);
    static void write_svg(const std::string& filename);
    static bool is_empty();
private:
    static std::chrono::time_point<std::chrono::steady_clock> global_start_time_;
    static std::map<std::thread::id, std::vector<TimeEvent>> events_;
    static thread_local size_t stack_size_;
    static size_t max_log_length_;
    static thread_local size_t event_id_;
    TimeEvent time_event_;
};

}
