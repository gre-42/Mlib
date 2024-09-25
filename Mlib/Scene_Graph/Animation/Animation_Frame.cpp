#include "Animation_Frame.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>
#include <shared_mutex>

namespace Mlib {

void AnimationFrame::advance_time(float dt, AnimationWrapMode wrap_mode) {
    if (!std::isnan(time)) {
        if (std::isnan(begin) != std::isnan(end)) {
            THROW_OR_ABORT("Inconsistent begin and end NaN-ness (0)");
        }
        if (!std::isnan(begin)) {
            if (end < begin) {
                THROW_OR_ABORT("Loop end before loop begin");
            }
            if (time < begin) {
                THROW_OR_ABORT("Loop time before loop begin");
            }
            if (time > end) {
                THROW_OR_ABORT("Loop time after loop end");
            }
            if (end == begin) {
                time = begin;
            } else {
                if (wrap_mode == AnimationWrapMode::PERIODIC) {
                    time = std::clamp(
                        begin + std::fmod(
                            time + dt - begin,
                            end - begin),
                        begin,
                        end);
                } else {
                    time = std::min(time + dt, end);
                }
            }
        }
    }
}

bool AnimationFrame::is_nan() const {
    if (std::isnan(begin) != std::isnan(end)) {
        THROW_OR_ABORT("Inconsistent begin and end NaN-ness (1)");
    }
    if (std::isnan(time) != std::isnan(end)) {
        THROW_OR_ABORT("Inconsistent begin and end NaN-ness (2)");
    }
    return std::isnan(time);
}

float PeriodicAnimationFrame::time() const {
    std::shared_lock lock{ *mutex_ };
    return frame_.time;
}

void PeriodicAnimationFrame::advance_time(float dt) {
    std::lock_guard lock{ *mutex_ };
    frame_.advance_time(dt, AnimationWrapMode::PERIODIC);
}

float AperiodicAnimationFrame::time() const {
    std::shared_lock lock{ *mutex_ };
    return frame_.time;
}

void AperiodicAnimationFrame::advance_time(float dt) {
    std::lock_guard lock{ *mutex_ };
    frame_.advance_time(dt, AnimationWrapMode::APERIODIC);
}

bool AperiodicAnimationFrame::active() const {
    if (std::isnan(frame_.begin) != std::isnan(frame_.end)) {
        THROW_OR_ABORT("Inconsistent begin and end NaN-ness (3)");
    }
    return !frame_.is_nan() && (frame_.time != frame_.end);
}

bool AperiodicAnimationFrame::ran_to_completion() const {
    return !frame_.is_nan() && (frame_.time == frame_.end);
}

float AperiodicAnimationFrame::duration() const {
    return frame_.end - frame_.begin;
}

float AperiodicAnimationFrame::elapsed() const {
    return frame_.time - frame_.begin;
}

bool AperiodicAnimationFrame::is_nan() const {
    return frame_.is_nan();
}

}
