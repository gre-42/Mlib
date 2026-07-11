#include "Audio_Device.hpp"
#ifndef USE_PCM_FILTERS
#include <Mlib/Audio/OpenALSoft_efx.h>
#endif
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <stdexcept>

using namespace Mlib;

ALCdevice *AudioDevice::device_ = nullptr;

AudioDevice::AudioDevice() {
    if (device_ != nullptr) {
        throw std::runtime_error("Audio device already opened");
    }
    // select the "preferred device"
    device_ = alcOpenDevice(nullptr);
    if (device_ == nullptr) {
        throw std::runtime_error("Could not open audio device");
    }
}

AudioDevice::~AudioDevice() {
    if (!alcCloseDevice(device_)) {
        verbose_abort("Could not close audio device");
    }
    device_ = nullptr;
}

unsigned int AudioDevice::get_frequency() {
    if (device_ == nullptr) {
        throw std::runtime_error("Audio device not initialized");
    }
    ALCint rate;
    alcGetIntegerv(device_, ALC_FREQUENCY, 1, &rate);
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        throw std::runtime_error("Could not read audio frequency, code: " + std::to_string(error));
    }
    return integral_cast<unsigned int>(rate);
}

std::string AudioDevice::get_name() {
    if (device_ == nullptr) {
        throw std::runtime_error("Audio device not initialized");
    }
    const auto* name = alcGetString(device_, ALC_DEVICE_SPECIFIER);
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        throw std::runtime_error("Could not read audio device name, code: " + std::to_string(error));
    }
    if (name == nullptr) {
        throw std::runtime_error("Could not read audio device name");
    }
    return name;
}

#ifndef USE_PCM_FILTERS
unsigned int AudioDevice::get_max_auxiliary_sends() {
    if (device_ == nullptr) {
        throw std::runtime_error("Audio device not initialized");
    }
    ALCint max_auxiliary_sends = 0;
    alcGetIntegerv(device_, ALC_MAX_AUXILIARY_SENDS, 1, &max_auxiliary_sends);
    if (ALCenum error = alcGetError(device_); error != ALC_NO_ERROR) {
        throw std::runtime_error("Could obtain max auxiliary sends per source, code: " + std::to_string(error));
    }
    return integral_cast<unsigned int>(max_auxiliary_sends);
}
#endif
