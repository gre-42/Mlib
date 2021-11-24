#include "Audio_Context.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <stdexcept>

using namespace Mlib;

AudioContext::AudioContext(AudioDevice& device) {
    context_ = alcCreateContext(device.device_, nullptr);
    if (context_ == nullptr) {
        throw std::runtime_error("Could not create audio context");
    }
    AL_CHK(alcMakeContextCurrent(context_));
}

AudioContext::~AudioContext() {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context_);
}
