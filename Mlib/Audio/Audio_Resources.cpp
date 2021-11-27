#include "Audio_Resources.hpp"
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <stdexcept>

using namespace Mlib;

void AudioResources::add_buffer(const std::string& name, const std::string& filename, float gain) {
    std::lock_guard lock{ mutex_ };
    if (!buffer_filenames_.insert({name, {filename, gain}}).second) {
        throw std::runtime_error("Audio buffer with name \"" + name + "\" already exists");
    }
}

float AudioResources::get_gain(const std::string& name) const {
    std::lock_guard lock{ mutex_ };
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        throw std::runtime_error("Unknown audio buffer: \"" + name + '"');
    }
    return fit->second.gain;
}

std::shared_ptr<AudioBuffer> AudioResources::get_buffer(const std::string& name) const {
    std::lock_guard lock{ mutex_ };
    {
        auto it = audio_buffers_.find(name);
        if (it != audio_buffers_.end()) {
            return it->second;
        }
    }
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        throw std::runtime_error("Unknown audio buffer: \"" + name + '"');
    }
    auto buffer = std::make_shared<AudioBuffer>();
    buffer->load_wave(fit->second.filename);
    if (!audio_buffers_.insert({name, buffer}).second) {
        throw std::runtime_error("Audio resources internal error");
    }
    return buffer;
}
