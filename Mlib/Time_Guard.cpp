#include "Time_Guard.hpp"
#include <Mlib/Images/Svg.hpp>
#include <fstream>
#include <iostream>

using namespace Mlib;

std::chrono::time_point<std::chrono::steady_clock> TimeGuard::global_start_time_;
std::map<std::thread::id, std::vector<TimeEvent>> TimeGuard::events_;
thread_local size_t TimeGuard::stack_size_ = 0;
size_t TimeGuard::max_log_length_ = 0;
thread_local size_t TimeGuard::event_id_ = 0;

void TimeGuard::set_max_log_length(size_t len) {
    max_log_length_ = len;
    global_start_time_ = std::chrono::steady_clock::now();
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
    std::map<std::thread::id, size_t> thread_num;
    for (const auto& e : events_) {
        thread_num[e.first] = thread_num.size();
    }
    if (false) {
        std::vector<float> x_start;
        std::vector<float> y_start;
        std::vector<float> x_stop;
        std::vector<float> y_stop;
        for (const auto& e : events_) {
            for (const auto& ee : e.second) {
                float start_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.start_time - global_start_time_).count();
                float end_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.end_time - global_start_time_).count();
                x_start.push_back(start_time);
                x_stop.push_back(end_time);
                y_start.push_back(ee.stack_size);
                y_stop.push_back(ee.stack_size);
            }
        }
        svg.plot_edges(x_start, y_start, x_stop, y_stop);
    }
    std::vector<float> x;
    std::vector<float> y;
    for (const auto& e : events_) {
        for (const auto& ee : e.second) {
            float start_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.start_time - global_start_time_).count();
            float end_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(ee.end_time - global_start_time_).count();
            x.push_back(start_time);
            x.push_back(end_time);
            y.push_back(ee.stack_size);
            y.push_back(ee.stack_size);
        }
    }
    svg.plot(x, y);
    svg.finish();
    ostr.flush();
    if (ostr.fail()) {
        throw std::runtime_error("Could not write " + filename);
    }
}

bool TimeGuard::is_empty() {
    return events_.empty();
}

TimeGuard::TimeGuard(const char* message)
: time_event_{
    .start_time = std::chrono::steady_clock::now(),
    .message = message,
    .stack_size = stack_size_}
{
    // std::cerr << message << std::endl;
    ++stack_size_;
    if (max_log_length_ == 0) {
        throw std::runtime_error("Please call \"TimeGuard::set_max_log_length\"");
    }
    auto& ar = events_[std::this_thread::get_id()];
    if (ar.capacity() == 0) {
        ar.reserve(max_log_length_);
    }
}

TimeGuard::~TimeGuard() {
    --stack_size_;
    time_event_.end_time = std::chrono::steady_clock::now();
    auto& ar = events_[std::this_thread::get_id()];
    if (event_id_ == ar.size()) {
        ar.push_back(time_event_);
    } else {
        ar[event_id_] = time_event_;
    }
    event_id_ = (event_id_ + 1) % max_log_length_;
}
