#include "Audio_Context.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

AudioContext::AudioContext(AudioDevice& device)
: device_{ device.device_ }
{
    context_ = alcCreateContext(device.device_, nullptr);
    if (context_ == nullptr) {
        throw std::runtime_error("Could not create audio context, code: " + std::to_string(alcGetError(device.device_)));
    }
    if (!alcMakeContextCurrent(context_)) {
        throw std::runtime_error("Could not make context current, code: " + std::to_string(alcGetError(device.device_)));
    }
}

AudioContext::~AudioContext() {
    if (!alcMakeContextCurrent(nullptr)) {
        std::cerr << "Could not remove current context, code: " << alcGetError(device_) << std::endl;
    }
    {
        alcDestroyContext(context_);
        ALCenum error = alcGetError(device_);
        if (error != ALC_NO_ERROR) {
            std::cerr << "Could not destroy context, code: " << error << std::endl;
        }
    }
}
