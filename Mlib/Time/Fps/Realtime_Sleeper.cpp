#include "Realtime_Sleeper.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Time/Busy_Sleep_Until.hpp>
#include <cmath>
#include <thread>

using namespace Mlib;

RealtimeSleeper::RealtimeSleeper(
    std::string prefix,
    float dt,
    float max_residual_time,
    bool print_residual_time)
    : dt_{ dt }
    , max_residual_time_{ max_residual_time }
    , print_residual_time_{ print_residual_time }
    , sim_time_{ std::chrono::steady_clock::now() }
    , prefix_{ std::move(prefix) }
    , is_up_to_date_{ true }
{}

RealtimeSleeper::~RealtimeSleeper() = default;

void RealtimeSleeper::tick() {
    sim_time_ += std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>{double(dt_)});
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration residual_time = sim_time_ - current_time;
    if (!std::isnan(max_residual_time_) && (std::chrono::duration<float>(residual_time).count() < -max_residual_time_)) {
        sim_time_ = current_time;
        if (print_residual_time_) {
            linfo() << prefix_ << "resetting sim time";
        }
        is_up_to_date_ = true;
    } else {
        if (residual_time.count() > 0) {
            is_up_to_date_ = true;
            // busy wait
            // while(residual_time.count() > 0) {
            //     current_time = std::chrono::steady_clock::now();
            //     residual_time = sim_time_ - current_time;
            // }
            // Mlib::sleep_for(residual_time);
            busy_sleep_until(sim_time_);
        } else {
            is_up_to_date_ = false;
            if (print_residual_time_) {
                linfo() <<
                    prefix_ <<
                    "residual time: " <<
                    std::chrono::duration_cast<std::chrono::milliseconds>(-residual_time).count() <<
                    " ms";
            }
        }
    }
}

void RealtimeSleeper::reset() {
    sim_time_ = std::chrono::steady_clock::now();
    if (print_residual_time_) {
        linfo() << prefix_ << "resetting sim time after pause";
    }
}

bool RealtimeSleeper::is_up_to_date() const {
    return is_up_to_date_;
}

void RealtimeSleeper::set_dt(float dt) {
    dt_ = dt;
}

std::chrono::steady_clock::time_point RealtimeSleeper::simulated_time() const {
    return sim_time_;
}
