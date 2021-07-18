#include "Animation_Frame.hpp"
#include <cmath>
#include <stdexcept>

namespace Mlib {

void AnimationFrame::advance_time(float dt, AnimationWrapMode wrap_mode) {
    if (!std::isnan(time)) {
        if (std::isnan(begin) != std::isnan(end)) {
            throw std::runtime_error("Inconsistent begin and end NAN-ness");
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

}
