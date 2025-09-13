#include "Audio_Context.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Audio/OpenAL_alc.h>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

AudioContext::AudioContext(AudioDevice& device, unsigned int frequency) {
    ALCint attrlist[] = {ALC_FREQUENCY, integral_cast<ALCint>(frequency), 0};
    auto *context = alcCreateContext(device.device_, frequency == 0 ? nullptr : attrlist);
    if (context == nullptr) {
        THROW_OR_ABORT("Could not create audio context, code: " +
                       std::to_string(alcGetError(device.device_)));
    }
    dgs_.add([context, d = device.device_]() {
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
    dgs_.add([d = device.device_]() {
        if (!alcMakeContextCurrent(nullptr)) {
            verbose_abort("Could not remove current context, code: " +
                          std::to_string(alcGetError(d)));
        }
    });
}

AudioContext::~AudioContext() = default;
