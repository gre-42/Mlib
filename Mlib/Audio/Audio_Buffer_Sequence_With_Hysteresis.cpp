#include "Audio_Buffer_Sequence_With_Hysteresis.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AudioBufferSequenceWithHysteresis::AudioBufferSequenceWithHysteresis(
    std::vector<AudioBufferAndFrequency> buffers,
    float hysteresis_step)
    : previous_result_{ nullptr }
    , hysteresis_step_{ hysteresis_step }
    , seq_{ std::move(buffers) }
{}

const AudioBufferAndFrequency& AudioBufferSequenceWithHysteresis::get_buffer_and_frequency(
    float frequency,
    PitchAdjustmentStrategy strategy)
{
    if (frequency < 0.f) {
        THROW_OR_ABORT("Requested negative audio frequency");
    }
    if ((previous_result_ == nullptr) ||
        (std::abs(previous_result_->frequency - frequency) / (previous_result_->frequency + 1e-6) > hysteresis_step_))
    {
        previous_result_ = &seq_.get_buffer_and_frequency(frequency, strategy);
    }
    return *previous_result_;
}
