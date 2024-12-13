#include "Time_Guard.hpp"
#include <Mlib/Images/Svg.hpp>
#include <fstream>
#include <iostream>
#include <set>

using namespace Mlib;

std::chrono::steady_clock::time_point TimeGuard::init_time_;
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
        THROW_OR_ABORT("No events recorder");
    }
    std::ofstream ostr{ filename };
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not open file " + filename);
    }
    Svg<double> svg{ ostr, 800, 600 };
    // if (false) {
    //     std::vector<double> x_start;
    //     std::vector<double> y_start;
    //     std::vector<double> x_stop;
    //     std::vector<double> y_stop;
    //     for (const auto& e : called_functions) {
    //         for (const auto& ee : e.second) {
    //             double start_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.start_time - global_start_time_).count();
    //             double end_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.end_time - global_start_time_).count();
    //             x_start.push_back(start_time);
    //             x_stop.push_back(end_time);
    //             y_start.push_back(ee.stack_size);
    //             y_stop.push_back(ee.stack_size);
    //         }
    //     }
    //     svg.plot_edges(x_start, y_start, x_stop, y_stop);
    // }
    std::vector<std::vector<double>> x(t->second.events.size());
    std::vector<std::vector<double>> y(t->second.events.size());
    size_t i = 0;
    for (const auto& e : thread_time_infos_) {
        std::set<TimeEvent> sorted_events(e.second.events.begin(), e.second.events.end());
        x[i].reserve(e.second.events.size());
        y[i].reserve(e.second.events.size());
        for (const auto& ee : sorted_events) {
            auto time = std::chrono::duration<double>(ee.time - init_time_).count();
            x[i].push_back(time);
            y[i].push_back((double)ee.stack_size);
        }
        ++i;
    }
    svg.plot_multiple(x, y, 0.1);
    svg.finish();
    ostr.flush();
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not write " + filename);
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
        THROW_OR_ABORT("Max log length exceeded");
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
        THROW_OR_ABORT("Max log length exceeded");
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
        THROW_OR_ABORT("Please call \"TimeGuard::initialize\"");
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
