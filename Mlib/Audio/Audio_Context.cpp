#include "Audio_Context.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#ifndef USE_PCM_FILTERS
#include <Mlib/Audio/OpenALSoft_efx.h>
#endif
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Audio/OpenAL_alc.h>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

AudioContext::AudioContext(
    AudioDevice& device,
    unsigned int frequency,
    unsigned int max_auxiliary_sends)
{
#ifndef USE_PCM_FILTERS
    if (alcIsExtensionPresent(device.device_, "ALC_EXT_EFX") == AL_FALSE) {
        throw std::runtime_error("OpenAL effects extension not present");
    }
#endif
    ALCint attrlist[] = {
        ALC_FREQUENCY,
        integral_cast<ALCint>(frequency),
        0,
#ifndef USE_PCM_FILTERS
        ALC_MAX_AUXILIARY_SENDS,
        integral_cast<ALCint>(max_auxiliary_sends)
#endif
    };
    auto *context = alcCreateContext(device.device_, frequency == 0 ? nullptr : attrlist);
    if (context == nullptr) {
        throw std::runtime_error("Could not create audio context, code: " +
                       std::to_string(alcGetError(device.device_)));
    }
    dgs_.add([context, d = device.device_]() {
        alcDestroyContext(context);
        if (ALCenum error = alcGetError(d); error != ALC_NO_ERROR) {
            verbose_abort("Could not destroy context, code: " + std::to_string(error));
        }
    });
    if (!alcMakeContextCurrent(context)) {
        throw std::runtime_error("Could not make context current, code: " +
                       std::to_string(alcGetError(device.device_)));
    }
    dgs_.add([d = device.device_]() {
        if (!alcMakeContextCurrent(nullptr)) {
            verbose_abort("Could not remove current context, code: " +
                          std::to_string(alcGetError(d)));
        }
    });
#ifndef USE_PCM_FILTERS
    linfo() << "Device supports " << device.get_max_auxiliary_sends() << " aux sends per source";
#endif
}

AudioContext::~AudioContext() = default;
