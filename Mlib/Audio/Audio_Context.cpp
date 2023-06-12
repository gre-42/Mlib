#include "Audio_Context.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
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
        THROW_OR_ABORT("Could not create audio context, code: " + std::to_string(alcGetError(device.device_)));
    }
    if (!alcMakeContextCurrent(context_)) {
        THROW_OR_ABORT("Could not make context current, code: " + std::to_string(alcGetError(device.device_)));
    }
}

AudioContext::~AudioContext() {
    if (!alcMakeContextCurrent(nullptr)) {
        verbose_abort("Could not remove current context, code: " + std::to_string(alcGetError(device_)));
    }
    {
        alcDestroyContext(context_);
        ALCenum error = alcGetError(device_);
        if (error != ALC_NO_ERROR) {
            verbose_abort("Could not destroy context, code: " + std::to_string(error));
        }
    }
}
