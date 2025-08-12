#pragma once
#include <Mlib/Audio/Audio_Buffer_Sequence.hpp>

namespace Mlib {

class AudioBufferSequenceWithHysteresis {
    AudioBufferSequenceWithHysteresis(const AudioBufferSequenceWithHysteresis &) = delete;
    AudioBufferSequenceWithHysteresis &operator=(const AudioBufferSequenceWithHysteresis &) = delete;

public:
    AudioBufferSequenceWithHysteresis(
        std::vector<AudioBufferAndFrequency> buffers,
        float hysteresis_step);
    const AudioBufferAndFrequency& get_buffer_and_frequency(
        float frequency,
        PitchAdjustmentStrategy strategy = PitchAdjustmentStrategy::UP_SAMPLING);

private:
    const AudioBufferAndFrequency* previous_result_;
    float hysteresis_step_;
    AudioBufferSequence seq_;
};

}
