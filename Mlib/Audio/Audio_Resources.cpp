#include "Audio_Resources.hpp"
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Buffer_Sequence_With_Hysteresis.hpp>
#include <Mlib/Audio/Audio_Equalizer.hpp>
#include <Mlib/Audio/Audio_File_Sequence.hpp>
#include <Mlib/Audio/Audio_Lowpass.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

AudioResources::AudioResources()
    : buffer_filenames_{"Buffer filename"}
    , buffer_meta_{"Buffer meta information"}
    , buffers_{"Buffer"}
    , buffer_sequence_filenames_{"Buffer sequence filename"}
    , buffer_sequences_{"Buffer sequence"}
    , equalizer_parameters_{"Equalizer parameters"}
    , equalizers_{"Equalizers"}
    , lowpass_parameters_{"Lowpass parameters"}
    , lowpasses_{"Lowpass"}
{}

AudioResources::~AudioResources() = default;

void AudioResources::add_buffer(
    const VariableAndHash<std::string>& name,
    const std::string& filename,
    float gain,
    const std::optional<Interval<float>>& distance_clamping,
    const std::optional<VariableAndHash<std::string>>& lowpass)
{
    std::unique_lock lock{ mutex_ };
    buffer_filenames_.add(
        name,
        filename,
        gain,
        distance_clamping,
        lowpass);
}

const AudioMetaInformation& AudioResources::get_buffer_meta(const VariableAndHash<std::string>& name) const {
    {
        std::shared_lock lock{ mutex_ };
        if (auto it = buffer_meta_.try_get(name); it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{ mutex_ };
    if (auto it = buffer_meta_.try_get(name); it != nullptr) {
        return *it;
    }
    const auto& file = buffer_filenames_.get(name);
    std::shared_ptr<AudioLowpass> lowpass;
    if (file.lowpass.has_value()) {
        lowpass = get_lowpass(*file.lowpass);
    }
    return buffer_meta_.add(name, file.gain, file.distance_clamping, lowpass);
}

std::shared_ptr<AudioBuffer> AudioResources::get_buffer(const VariableAndHash<std::string>& name) const {
    {
        std::shared_lock lock{ mutex_ };
        if (auto it = buffers_.try_get(name); it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{ mutex_ };
    if (auto it = buffers_.try_get(name); it != nullptr) {
        return *it;
    }
    auto& fit = buffer_filenames_.get(name);
    auto buffer = AudioBuffer::from_wave(fit.filename);
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
        if (auto it = buffer_sequences_.try_get(name); it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{mutex_};
    if (auto it = buffer_sequences_.try_get(name); it != nullptr) {
        return *it;
    }
    auto& it = buffer_sequence_filenames_.get(name);
    auto items = load_audio_file_sequence(it.filename);
    std::list<AudioBufferAndFrequency> buffers;
    for (const auto& i : items) {
        buffers.push_back(AudioBufferAndFrequency{
            .buffer = AudioBuffer::from_wave(i.filename),
            .frequency = i.frequency});
    }
    auto seq = std::make_shared<AudioBufferSequenceWithHysteresis>(std::vector(buffers.begin(), buffers.end()), it.hysteresis_step);
    buffer_sequences_.add(name, seq);
    return seq;
}

void AudioResources::add_equalizer(
    const VariableAndHash<std::string>& name,
    const AudioEqualizerInformation& equalizer) const
{
    std::shared_lock lock{mutex_};
    equalizer_parameters_.add(name, equalizer);
}

std::shared_ptr<AudioEqualizer>
    AudioResources::get_equalizer(const VariableAndHash<std::string>& name) const
{
    {
        std::shared_lock lock{mutex_};
        if (auto it = equalizers_.try_get(name); it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{mutex_};
    if (auto it = equalizers_.try_get(name); it != nullptr) {
        return *it;
    }
    auto equalizer = AudioEqualizer::create(equalizer_parameters_.get(name));
    return equalizers_.add(name, equalizer);
}

void AudioResources::add_lowpass(
    const VariableAndHash<std::string>& name,
    const AudioLowpassInformation& lowpass) const
{
    std::shared_lock lock{mutex_};
    lowpass_parameters_.add(name, lowpass);
}

std::shared_ptr<AudioLowpass>
    AudioResources::get_lowpass(const VariableAndHash<std::string>& name) const
{
    {
        std::shared_lock lock{mutex_};
        if (auto it = lowpasses_.try_get(name); it != nullptr) {
            return *it;
        }
    }
    std::unique_lock lock{mutex_};
    if (auto it = lowpasses_.try_get(name); it != nullptr) {
        return *it;
    }
    auto lowpass = AudioLowpass::create(lowpass_parameters_.get(name));
    return lowpasses_.add(name, lowpass);
}
