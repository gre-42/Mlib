#pragma once
#include <chrono>
#include <map>
#include <thread>
#include <vector>

namespace Mlib {

template <class TSize>
class Svg;

struct TimeEvent {
    size_t event_id;
    std::chrono::time_point<std::chrono::steady_clock> time;
    const char* message = nullptr;
    size_t stack_size = SIZE_MAX;
    std::strong_ordering operator <=> (const TimeEvent& other) const {
        return event_id <=> other.event_id;
    }
};

struct CalledFunction {
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
    static void insert_event(const TimeEvent& e);
    static void insert_called_function(const CalledFunction& e);
    static std::chrono::time_point<std::chrono::steady_clock> global_start_time_;
    static std::map<std::thread::id, std::vector<TimeEvent>> events_;
    static std::map<std::thread::id, std::vector<CalledFunction>> called_functions_;
    static thread_local size_t stack_size_;
    static size_t max_log_length_;
    static thread_local size_t event_id_;
    static thread_local size_t called_function_id_;
    CalledFunction called_function_;
};

}
