#include "Audio_Resources.hpp"
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Buffer_Sequence_With_Hysteresis.hpp>
#include <Mlib/Audio/Audio_File_Sequence.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

AudioResources::AudioResources()
    : buffer_filenames_{"Buffer filename"}
    , buffers_{"Buffer"}
    , buffer_sequence_filenames_{"Buffer sequence filename"}
    , buffer_sequences_{"Buffer sequence"}
{}

AudioResources::~AudioResources() = default;

void AudioResources::add_buffer(
    const VariableAndHash<std::string>& name,
    const std::string& filename,
    float gain,
    const std::optional<Interval<float>>& distance_clamping)
{
    std::unique_lock lock{ mutex_ };
    buffer_filenames_.add(
        name,
        filename,
        gain,
        distance_clamping);
}

const AudioFileInformation& AudioResources::get_buffer_meta(const VariableAndHash<std::string>& name) const {
    std::shared_lock lock{ mutex_ };
    return buffer_filenames_.get(name);
}

std::shared_ptr<AudioBuffer> AudioResources::get_buffer(const VariableAndHash<std::string>& name) const {
    {
        std::shared_lock lock{ mutex_ };
        auto it = buffers_.try_get(name);
        if (it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{ mutex_ };
    auto& fit = buffer_filenames_.get(name);
    auto buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(fit.filename));
    buffers_.add(name, buffer);
    return buffer;
}

void AudioResources::preload_buffer(const VariableAndHash<std::string>& name) const {
    get_buffer(name);
}

void AudioResources::add_buffer_sequence(
    const VariableAndHash<std::string>& name,
    const std::string& filename,
    float gain,
    const std::optional<Interval<float>>& distance_clamping,
    float hysteresis_step)
{
    if (gain < 0.f) {
        THROW_OR_ABORT("Attempt to set negative buffer sequence gain");
    }
    if (gain > 1.f) {
        THROW_OR_ABORT("Attempt to set buffer sequence gain greater than 1");
    }
    std::unique_lock lock{mutex_};
    buffer_sequence_filenames_.add(name, filename, gain, distance_clamping, hysteresis_step);
}

float AudioResources::get_buffer_sequence_gain(const VariableAndHash<std::string>& name) const {
    std::shared_lock lock{mutex_};
    return buffer_sequence_filenames_.get(name).gain;
}

std::shared_ptr<AudioBufferSequenceWithHysteresis> AudioResources::get_buffer_sequence(const VariableAndHash<std::string>& name) const {
    {
        std::shared_lock lock{mutex_};
        auto it = buffer_sequences_.try_get(name);
        if (it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{mutex_};
    auto& it = buffer_sequence_filenames_.get(name);
    auto items = load_audio_file_sequence(it.filename);
    std::list<AudioBufferAndFrequency> buffers;
    for (const auto& i : items) {
        buffers.push_back(AudioBufferAndFrequency{
            .buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(i.filename)),
            .frequency = i.frequency});
    }
    auto seq = std::make_shared<AudioBufferSequenceWithHysteresis>(std::vector(buffers.begin(), buffers.end()), it.hysteresis_step);
    buffer_sequences_.add(name, seq);
    return seq;
}
