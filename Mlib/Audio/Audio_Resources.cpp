#include "Audio_Resources.hpp"
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Buffer_Sequence_With_Hysteresis.hpp>
#include <Mlib/Audio/Audio_File_Sequence.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

AudioResources::AudioResources() = default;

AudioResources::~AudioResources() = default;

void AudioResources::add_buffer(const std::string& name, const std::string& filename, float gain) {
    std::unique_lock lock{ mutex_ };
    if (!buffer_filenames_.insert({name, {filename, gain}}).second) {
        THROW_OR_ABORT("Audio buffer with name \"" + name + "\" already exists");
    }
}

float AudioResources::get_buffer_gain(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        THROW_OR_ABORT("Unknown audio buffer: \"" + name + '"');
    }
    return fit->second.gain;
}

std::shared_ptr<AudioBuffer> AudioResources::get_buffer(const std::string& name) const {
    {
        std::shared_lock lock{ mutex_ };
        auto it = buffers_.find(name);
        if (it != buffers_.end()) {
            return it->second;
        }
    }
    std::unique_lock lock{ mutex_ };
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        THROW_OR_ABORT("Unknown audio buffer: \"" + name + '"');
    }
    auto buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(fit->second.filename));
    if (!buffers_.insert({name, buffer}).second) {
        THROW_OR_ABORT("Audio resources internal error");
    }
    return buffer;
}

void AudioResources::add_buffer_sequence(
    const std::string& name,
    const std::string& filename,
    float gain,
    float hysteresis_step)
{
    if (gain < 0.f) {
        THROW_OR_ABORT("Attempt to set negative buffer sequence gain");
    }
    if (gain > 1.f) {
        THROW_OR_ABORT("Attempt to set buffer sequence gain greater than 1");
    }
    std::unique_lock lock{mutex_};
    if (!buffer_sequence_filenames_.insert({name, {filename, gain, hysteresis_step}}).second) {
        THROW_OR_ABORT("Audio sequence with name \"" + name + "\" already exists");
    }
}

float AudioResources::get_buffer_sequence_gain(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = buffer_sequence_filenames_.find(name);
    if (it == buffer_sequence_filenames_.end()) {
        THROW_OR_ABORT("Could not find audio sequence with name \"" + name + '"');
    }
    return it->second.gain;
}

std::shared_ptr<AudioBufferSequenceWithHysteresis> AudioResources::get_buffer_sequence(const std::string& name) const {
    {
        std::shared_lock lock{mutex_};
        auto it = buffer_sequences_.find(name);
        if (it != buffer_sequences_.end()) {
            return it->second;
        }
    }
    std::unique_lock lock{mutex_};
    auto it = buffer_sequence_filenames_.find(name);
    if (it == buffer_sequence_filenames_.end()) {
        THROW_OR_ABORT("Could not find audio sequence with name \"" + name + '"');
    }
    auto items = load_audio_file_sequence(it->second.filename);
    std::list<AudioBufferAndFrequency> buffers;
    for (const auto& i : items) {
        buffers.push_back(AudioBufferAndFrequency{
            .buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(i.filename)),
            .frequency = i.frequency});
    }
    auto seq = std::make_shared<AudioBufferSequenceWithHysteresis>(std::vector(buffers.begin(), buffers.end()), it->second.hysteresis_step);
    if (!buffer_sequences_.insert({name, seq}).second) {
        THROW_OR_ABORT("Could not insert audio buffer sequence \"" + name + '"');
    }
    return seq;
}
