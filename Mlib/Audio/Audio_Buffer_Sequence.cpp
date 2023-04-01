#include "Audio_Buffer_Sequence.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>

using namespace Mlib;

AudioBufferSequence::AudioBufferSequence(std::vector<AudioBufferAndFrequency> buffers)
: buffers_{std::move(buffers)}
{}

const AudioBufferAndFrequency& AudioBufferSequence::get_buffer_and_frequency(float frequency) const {
    if (buffers_.empty()) {
        THROW_OR_ABORT("Audio buffer vector is empty");
    }
    auto it = std::lower_bound(
        buffers_.begin(),
        buffers_.end(),
        frequency,
        [](const AudioBufferAndFrequency& l, float f){return l.frequency < f;});
    if (it == buffers_.end()) {
        return buffers_.back();
    }
    return *it;
}
