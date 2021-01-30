#pragma once
#include <chrono>
#include <iosfwd>
#include <map>
#include <string>
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
    std::string group;
    size_t stack_size = SIZE_MAX;
};

struct ThreadTimeInfo {
    std::vector<TimeEvent> events;
    std::vector<CalledFunction> called_functions;
    size_t stack_size = 0;
    size_t event_id = 0;
    size_t called_function_id = 0;
};

enum class MaxLogLengthExceededBehavior {
    THROW_EXCEPTION,
    PERIODIC
};

class TimeGuard {
public:
    TimeGuard(const char* message, const std::string& group);
    ~TimeGuard();
    static void initialize(
        size_t max_log_length,
        MaxLogLengthExceededBehavior max_log_length_exceeded_behavior);
    static void write_svg(const std::thread::id& tid, const std::string& filename);
    static void print_groups(std::ostream& ostr);
    static bool is_empty(const std::thread::id& tid);
private:
    static void insert_event(const TimeEvent& e);
    static void insert_called_function(const CalledFunction& e);
    static std::chrono::time_point<std::chrono::steady_clock> init_time_;
    static std::map<std::thread::id, ThreadTimeInfo> thread_time_infos_;
    static size_t max_log_length_;
    static MaxLogLengthExceededBehavior max_log_length_exceeded_behavior_;
    CalledFunction called_function_;
};

}
