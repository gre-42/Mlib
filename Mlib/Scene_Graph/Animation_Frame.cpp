#include "Animation_Frame.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Mlib {

void AnimationFrame::advance_time(float dt, AnimationWrapMode wrap_mode) {
    if (!std::isnan(time)) {
        if (std::isnan(begin) != std::isnan(end)) {
            throw std::runtime_error("Inconsistent begin and end NAN-ness (0)");
        }
        if (!std::isnan(begin)) {
            if (end < begin) {
                throw std::runtime_error("Loop end before loop begin");
            }
            if (time < begin) {
                throw std::runtime_error("Loop time before loop begin");
            }
            if (time > end) {
                throw std::runtime_error("Loop time after loop end");
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
        throw std::runtime_error("Inconsistent begin and end NAN-ness (1)");
    }
    if (std::isnan(time) != std::isnan(end)) {
        throw std::runtime_error("Inconsistent begin and end NAN-ness (2)");
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
        throw std::runtime_error("Inconsistent begin and end NAN-ness (3)");
    }
    return !frame.is_nan() && (frame.time != frame.end);
}

bool AperiodicAnimationFrame::ran_to_completion() const {
    return !frame.is_nan() && (frame.time == frame.end);
}

}
