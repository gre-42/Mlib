#include "Audio_Context.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Audio/OpenAL_alc.h>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

AudioContext::AudioContext(AudioDevice &device) {
    auto *context = alcCreateContext(device.device_, nullptr);
    if (context == nullptr) {
        THROW_OR_ABORT("Could not create audio context, code: " +
                       std::to_string(alcGetError(device.device_)));
    }
    dgs_.add([this, context, d = device.device_]() {
        alcDestroyContext(context);
        ALCenum error = alcGetError(d);
        if (error != ALC_NO_ERROR) {
            verbose_abort("Could not destroy context, code: " + std::to_string(error));
        }
    });
    if (!alcMakeContextCurrent(context)) {
        THROW_OR_ABORT("Could not make context current, code: " +
                       std::to_string(alcGetError(device.device_)));
    }
    dgs_.add([this, d = device.device_]() {
        if (!alcMakeContextCurrent(nullptr)) {
            verbose_abort("Could not remove current context, code: " +
                          std::to_string(alcGetError(d)));
        }
    });
}

AudioContext::~AudioContext() = default;
