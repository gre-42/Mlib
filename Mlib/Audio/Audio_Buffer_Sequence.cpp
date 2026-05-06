#include "Audio_Buffer_Sequence.hpp"
#include <algorithm>
#include <stdexcept>

using namespace Mlib;

PitchAdjustmentStrategy Mlib::pitch_adjustment_strategy_from_string(const std::string& s) {
    if (s == "rounding") {
        return PitchAdjustmentStrategy::ROUNDING;
    }
    if (s == "up_sampling") {
        return PitchAdjustmentStrategy::UP_SAMPLING;
    }
    if (s == "down_sampling") {
        return PitchAdjustmentStrategy::DOWN_SAMPLING;
    }
    throw std::runtime_error("Unknown pitch adjustment strategy: \"" + s + '"');
}

AudioBufferSequence::AudioBufferSequence(std::vector<AudioBufferAndFrequency> buffers)
: buffers_{std::move(buffers)}
{}

const AudioBufferAndFrequency& AudioBufferSequence::get_buffer_and_frequency(
    float frequency,
    PitchAdjustmentStrategy strategy) const
{
    if (buffers_.empty()) {
        throw std::runtime_error("Audio buffer vector is empty");
    }
    auto it = std::lower_bound(
        buffers_.begin(),
        buffers_.end(),
        frequency,
        [](const AudioBufferAndFrequency& l, float f){return l.frequency < f;});
    if (it == buffers_.end()) {
        return buffers_.back();
    }
    if (it == buffers_.begin()) {
        return *it;
    }
    if (strategy == PitchAdjustmentStrategy::ROUNDING) {
        float d_left = frequency - (it - 1)->frequency;
        float d_right = it->frequency - frequency;
        if (d_left < d_right) {
            return *(it - 1);
        } else {
            return *it;
        }
    }
    if (strategy == PitchAdjustmentStrategy::UP_SAMPLING) {
        return *it;
    }
    if (strategy == PitchAdjustmentStrategy::DOWN_SAMPLING) {
        return *(it - 1);
    }
    throw std::runtime_error("Unknown pitch adjustment strategy");
}
