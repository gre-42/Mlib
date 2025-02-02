#include "Measure_Fps.hpp"
#include <Mlib/Os/Os.hpp>
#include <cmath>

using namespace Mlib;

MeasureFps::MeasureFps(double alpha, unsigned int print_interval)
    : last_dt_{ NAN }
    , mean_dt_{ 0 }
    , mad_dt_{ 0 }
    , alpha_{ alpha }
    , print_counter_{ 0 }
    , print_interval_{ print_interval }
{}

MeasureFps::~MeasureFps() = default;

void MeasureFps::tick() {
    auto current_time = std::chrono::steady_clock::now();
    if (last_time_ != std::chrono::steady_clock::time_point()) {
        auto dt = std::chrono::duration<double>(current_time - last_time_).count();
        mean_dt_ = (1 - alpha_) * mean_dt_ + alpha_ * dt;
        mad_dt_ = (1 - alpha_) * mad_dt_ + alpha_ * std::abs(mean_dt_ - dt);
        if (print_interval_ != UINT_MAX) {
            print_counter_ = (print_counter_ + 1) % print_interval_;
            if (print_counter_ == 0) {
                // linfo() << "FPS: " << min_fps() << " - " << max_fps();
                linfo() << "FPS: Mean = " << mean_fps() << ", MAD = " << mad_fps();
            }
        }
    }
    last_time_ = current_time;
}

bool MeasureFps::is_up_to_date() const {
    return true;
}

void MeasureFps::reset() {
    *this = MeasureFps{ alpha_, print_counter_ };
}

double MeasureFps::mean_dt() const {
    return mean_dt_;
}

double MeasureFps::last_fps() const {
    return 1 / last_dt_;
}

double MeasureFps::mean_fps() const {
    return 1 / mean_dt_;
}

double MeasureFps::mad_fps() const {
    return (max_fps() - min_fps()) / 2.;
}

double MeasureFps::min_fps() const {
    return 1.f / (mean_dt_ + mad_dt_);
}

double MeasureFps::max_fps() const {
    return 1.f / (mean_dt_ - mad_dt_);
}
