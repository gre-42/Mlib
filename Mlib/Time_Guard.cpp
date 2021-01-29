#include "Time_Guard.hpp"
#include <Mlib/Images/Svg.hpp>
#include <fstream>
#include <iostream>
#include <set>

using namespace Mlib;

std::chrono::time_point<std::chrono::steady_clock> TimeGuard::global_start_time_;
std::map<std::thread::id, std::vector<TimeEvent>> TimeGuard::events_;
std::map<std::thread::id, std::vector<CalledFunction>> TimeGuard::called_functions_;
thread_local size_t TimeGuard::stack_size_ = 0;
size_t TimeGuard::max_log_length_ = 0;
thread_local size_t TimeGuard::event_id_ = 0;
thread_local size_t TimeGuard::called_function_id_ = 0;

void TimeGuard::initialize(size_t max_log_length) {
    global_start_time_ = std::chrono::steady_clock::now();
    events_.clear();
    called_functions_.clear();
    stack_size_ = 0;
    max_log_length_ = max_log_length;
    event_id_ = 0;
    called_function_id_ = 0;
}

void TimeGuard::write_svg(const std::string& filename) {
    if (events_.empty()) {
        throw std::runtime_error("Events array is empty");
    }
    std::ofstream ostr{filename};
    if (ostr.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    Svg<float> svg{ostr, 800, 600};
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
    std::vector<std::vector<float>> x(events_.size());
    std::vector<std::vector<float>> y(events_.size());
    size_t i = 0;
    for (const auto& e : events_) {
        std::set<TimeEvent> sorted_events(e.second.begin(), e.second.end());
        x[i].reserve(e.second.size());
        y[i].reserve(e.second.size());
        for (const auto& ee : sorted_events) {
            float time = 1e-6f * std::chrono::duration_cast<std::chrono::microseconds>(ee.time - global_start_time_).count();
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
    std::map<std::thread::id, std::map<std::string, NAndDuration>> durations;
    for (const auto& t : called_functions_) {
        auto& d = durations[t.first];
        for (const auto& f : t.second) {
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
}

bool TimeGuard::is_empty() {
    return events_.empty();
}

void TimeGuard::insert_event(const TimeEvent& e) {
    auto& ar = events_[std::this_thread::get_id()];
    if (event_id_ < max_log_length_) {
        ar.push_back(e);
    } else {
        ar[event_id_ % max_log_length_] = e;
    }
    ++event_id_;
}

void TimeGuard::insert_called_function(const CalledFunction& called_function) {
    auto& ar = called_functions_[std::this_thread::get_id()];
    if (called_function_id_ < max_log_length_) {
        ar.push_back(called_function);
    } else {
        ar[called_function_id_ % max_log_length_] = called_function;
    }
    ++called_function_id_;
}

TimeGuard::TimeGuard(const char* message, const std::string& group)
: called_function_{
    .start_time = std::chrono::steady_clock::now(),
    .message = message,
    .group = group,
    .stack_size = stack_size_}
{
    if (max_log_length_ == 0) {
        throw std::runtime_error("Please call \"TimeGuard::initialize\"");
    }
    auto& ar = events_[std::this_thread::get_id()];
    if (ar.capacity() == 0) {
        ar.reserve(max_log_length_);
    }
    insert_event({
        .event_id = event_id_,
        .time = std::chrono::steady_clock::now(),
        .message = message,
        .stack_size = stack_size_});
    ++stack_size_;
    insert_event({
        .event_id = event_id_,
        .time = std::chrono::steady_clock::now(),
        .message = message,
        .stack_size = stack_size_});
}

TimeGuard::~TimeGuard() {
    insert_event({
        .event_id = event_id_,
        .time = std::chrono::steady_clock::now(),
        .message = "dtor",
        .stack_size = stack_size_});
    --stack_size_;
    insert_event({
        .event_id = event_id_,
        .time = std::chrono::steady_clock::now(),
        .message = "dtor",
        .stack_size = stack_size_});
    called_function_.end_time = std::chrono::steady_clock::now();
    insert_called_function(called_function_);
}
