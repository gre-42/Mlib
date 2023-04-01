#include "Audio_Resources.hpp"
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void AudioResources::add_buffer(const std::string& name, const std::string& filename, float gain) {
    std::scoped_lock lock{ mutex_ };
    if (!buffer_filenames_.insert({name, {filename, gain}}).second) {
        THROW_OR_ABORT("Audio buffer with name \"" + name + "\" already exists");
    }
}

float AudioResources::get_gain(const std::string& name) const {
    std::scoped_lock lock{ mutex_ };
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        THROW_OR_ABORT("Unknown audio buffer: \"" + name + '"');
    }
    return fit->second.gain;
}

std::shared_ptr<AudioBuffer> AudioResources::get_buffer(const std::string& name) const {
    std::scoped_lock lock{ mutex_ };
    {
        auto it = audio_buffers_.find(name);
        if (it != audio_buffers_.end()) {
            return it->second;
        }
    }
    auto fit = buffer_filenames_.find(name);
    if (fit == buffer_filenames_.end()) {
        THROW_OR_ABORT("Unknown audio buffer: \"" + name + '"');
    }
    auto buffer = std::make_shared<AudioBuffer>(AudioBuffer::from_wave(fit->second.filename));
    if (!audio_buffers_.insert({name, buffer}).second) {
        THROW_OR_ABORT("Audio resources internal error");
    }
    return buffer;
}
