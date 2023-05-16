#include "Animation_Frame.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cmath>

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

void PeriodicAnimationFrame::advance_time(float dt) {
    frame.advance_time(dt, AnimationWrapMode::PERIODIC);
}

void AperiodicAnimationFrame::advance_time(float dt) {
    frame.advance_time(dt, AnimationWrapMode::APERIODIC);
}

bool AperiodicAnimationFrame::active() const {
    if (std::isnan(frame.begin) != std::isnan(frame.end)) {
        THROW_OR_ABORT("Inconsistent begin and end NaN-ness (3)");
    }
    return !frame.is_nan() && (frame.time != frame.end);
}

bool AperiodicAnimationFrame::ran_to_completion() const {
    return !frame.is_nan() && (frame.time == frame.end);
}

}
