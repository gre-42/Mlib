#include "Time_Guard.hpp"
#include <Mlib/Images/Svg.hpp>
#include <fstream>
#include <iostream>
#include <set>

using namespace Mlib;

std::chrono::time_point<std::chrono::steady_clock> TimeGuard::init_time_;
std::map<std::thread::id, ThreadTimeInfo> TimeGuard::thread_time_infos_;
size_t TimeGuard::max_log_length_ = 0;
MaxLogLengthExceededBehavior TimeGuard::max_log_length_exceeded_behavior_;

void TimeGuard::initialize(
    size_t max_log_length,
    MaxLogLengthExceededBehavior max_log_length_exceeded_behavior)
{
    init_time_ = std::chrono::steady_clock::now();
    thread_time_infos_.clear();
    max_log_length_ = max_log_length;
    max_log_length_exceeded_behavior_ = max_log_length_exceeded_behavior;
}

void TimeGuard::write_svg(const std::thread::id& tid, const std::string& filename) {
    const auto& t = thread_time_infos_.find(tid);
    if (t == thread_time_infos_.end()) {
        throw std::runtime_error("No events recorder");
    }
    std::ofstream ostr{ filename };
    if (ostr.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    Svg<float> svg{ ostr, 800, 600 };
    // if (false) {
    //     std::vector<float> x_start;
    //     std::vector<float> y_start;
    //     std::vector<float> x_stop;
    //     std::vector<float> y_stop;
    //     for (const auto& e : called_functions) {
    //         for (const auto& ee : e.second) {
    //             float start_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.start_time - global_start_time_).count();
    //             float end_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.end_time - global_start_time_).count();
    //             x_start.push_back(start_time);
    //             x_stop.push_back(end_time);
    //             y_start.push_back(ee.stack_size);
    //             y_stop.push_back(ee.stack_size);
    //         }
    //     }
    //     svg.plot_edges(x_start, y_start, x_stop, y_stop);
    // }
    std::vector<std::vector<float>> x(t->second.events.size());
    std::vector<std::vector<float>> y(t->second.events.size());
    size_t i = 0;
    for (const auto& e : thread_time_infos_) {
        std::set<TimeEvent> sorted_events(e.second.events.begin(), e.second.events.end());
        x[i].reserve(e.second.events.size());
        y[i].reserve(e.second.events.size());
        for (const auto& ee : sorted_events) {
            float time = 1e-6f * std::chrono::duration_cast<std::chrono::microseconds>(ee.time - init_time_).count();
            x[i].push_back(time);
            y[i].push_back((float)ee.stack_size);
        }
        ++i;
    }
    svg.plot(x, y, 0.1f);
    svg.finish();
    ostr.flush();
    if (ostr.fail()) {
        throw std::runtime_error("Could not write " + filename);
    }
}

struct NAndDuration {
    size_t n = 0;
    std::chrono::duration<double, std::milli> total_duration{ 0 };
};

void TimeGuard::print_groups(std::ostream& ostr) {
    std::chrono::duration<double, std::milli> time_since_init = std::chrono::steady_clock::now() - init_time_;
    std::map<std::thread::id, std::map<std::string, NAndDuration>> durations;
    for (const auto& t : thread_time_infos_) {
        auto& d = durations[t.first];
        for (const auto& f : t.second.called_functions) {
            std::chrono::duration<double, std::milli> dt = (f.end_time - f.start_time);
            d[f.group].total_duration += dt;
            ++d[f.group].n;
        }
    }
    for (const auto& t : durations) {
        ostr << "Thread: " << t.first << '\n';
        std::chrono::duration<double, std::milli> total{ 0 };
        for (const auto& d : t.second) {
            ostr << "  " << d.second.total_duration.count() << " (" << d.second.n << ") " << d.first << '\n';
            total += d.second.total_duration;
        }
        ostr << "Total: " << total.count() << '\n';
    }
    ostr << "Time since init: " << time_since_init.count() << '\n';
}

bool TimeGuard::is_empty(const std::thread::id& tid) {
    return thread_time_infos_.find(tid) == thread_time_infos_.end();
}

void TimeGuard::insert_event(const TimeEvent& e) {
    auto& ar = thread_time_infos_[std::this_thread::get_id()];
    if (ar.event_id < max_log_length_) {
        ar.events.push_back(e);
    } else if (max_log_length_exceeded_behavior_ == MaxLogLengthExceededBehavior::THROW_EXCEPTION) {
            throw std::runtime_error("Max log length exceeded");
    } else {
        ar.events[ar.event_id % max_log_length_] = e;
    }
    ++ar.event_id;
}

void TimeGuard::insert_called_function(const CalledFunction& called_function) {
    auto& ar = thread_time_infos_[std::this_thread::get_id()];
    if (ar.called_function_id < max_log_length_) {
        ar.called_functions.push_back(called_function);
    } else if (max_log_length_exceeded_behavior_ == MaxLogLengthExceededBehavior::THROW_EXCEPTION) {
            throw std::runtime_error("Max log length exceeded");
    } else {
        ar.called_functions[ar.called_function_id % max_log_length_] = called_function;
    }
    ++ar.called_function_id;
}

TimeGuard::TimeGuard(const char* message, const std::string& group)
: called_function_{
    .start_time = std::chrono::steady_clock::now(),
    .message = message,
    .group = group,
    .stack_size = thread_time_infos_[std::this_thread::get_id()].stack_size}
{
    if (max_log_length_ == 0) {
        throw std::runtime_error("Please call \"TimeGuard::initialize\"");
    }
    auto& ar = thread_time_infos_[std::this_thread::get_id()];
    if (ar.events.capacity() == 0) {
        ar.events.reserve(max_log_length_);
    }
    insert_event({
        .event_id = ar.event_id,
        .time = std::chrono::steady_clock::now(),
        .message = message,
        .stack_size = ar.stack_size});
    ++ar.stack_size;
    insert_event({
        .event_id = ar.event_id,
        .time = std::chrono::steady_clock::now(),
        .message = message,
        .stack_size = ar.stack_size});
}

TimeGuard::~TimeGuard() {
    auto& ar = thread_time_infos_[std::this_thread::get_id()];
    insert_event({
        .event_id = ar.event_id,
        .time = std::chrono::steady_clock::now(),
        .message = "dtor",
        .stack_size = ar.stack_size});
    --ar.stack_size;
    insert_event({
        .event_id = ar.event_id,
        .time = std::chrono::steady_clock::now(),
        .message = "dtor",
        .stack_size = ar.stack_size});
    called_function_.end_time = std::chrono::steady_clock::now();
    insert_called_function(called_function_);
}
