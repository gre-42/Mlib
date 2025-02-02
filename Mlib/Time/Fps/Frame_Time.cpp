#include "Frame_Time.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

FrameTime::FrameTime(
    std::chrono::steady_clock::duration delay,
    std::function<std::chrono::steady_clock::duration()> dt,
    float alpha)
    : delay_{ delay }
    , dt_{ std::move(dt) }
    , alpha_{ alpha }
{}

FrameTime::~FrameTime() = default;

void FrameTime::tick() {
    if (smooth_time_ == std::chrono::steady_clock::time_point()) {
        smooth_time_ = std::chrono::steady_clock::now();
    } else {
        smooth_time_ += dt_();
        smooth_time_ -= std::chrono::steady_clock::duration{
            (std::chrono::steady_clock::rep)
            std::round(double(alpha_) * double((smooth_time_ - std::chrono::steady_clock::now()).count())) };
        // This is now replaced with "SetFps::completed_time()".
        // smooth_time_ = std::min(smooth_time_, std::chrono::steady_clock::now());
    }
}

void FrameTime::reset() {
    smooth_time_ = std::chrono::steady_clock::time_point();
}

bool FrameTime::is_up_to_date() const {
    return true;
}

std::chrono::steady_clock::time_point FrameTime::frame_time() const {
    if (smooth_time_ == std::chrono::steady_clock::time_point()) {
        THROW_OR_ABORT("FrameTime::frame_time on uninitialized object");
    }
    return smooth_time_ - delay_;
}
