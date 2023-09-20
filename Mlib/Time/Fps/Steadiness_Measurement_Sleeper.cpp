#include "Steadiness_Measurement_Sleeper.hpp"
#include <Mlib/Os/Os.hpp>
#include <thread>

using namespace Mlib;

SteadinessMeasurementSleeper::SteadinessMeasurementSleeper(float alpha, unsigned int print_interval)
    : print_counter_{0}
    , print_interval_{print_interval}
    , mean_{0.f}
    , deviation_{0.f}
    , alpha_{alpha}
{}

SteadinessMeasurementSleeper::~SteadinessMeasurementSleeper() = default;

void SteadinessMeasurementSleeper::tick() {
    auto time_new = std::chrono::steady_clock::now();
    if (time_old_ != std::chrono::steady_clock::time_point()) {
        auto dt = std::chrono::duration<float>(time_new - time_old_).count();
        mean_ = (1 - alpha_) * mean_ + alpha_ * dt;
        deviation_ = (1 - alpha_) * deviation_ + alpha_ * std::abs(mean_ - dt);
        if ((print_interval_ != UINT_MAX) && (print_counter_++ % print_interval_ == 0)) {
            linfo() << "FPS range: " << 1.f / (mean_ + deviation_) << " - "
                    << 1.f / (mean_ - deviation_);
        }
    }
    time_old_ = time_new;
}

void SteadinessMeasurementSleeper::reset() {
    time_old_ = std::chrono::steady_clock::time_point();
    mean_ = 0.f;
    print_counter_ = 0;
}

bool SteadinessMeasurementSleeper::is_up_to_date() const {
    return true;
}
