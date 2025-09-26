#include "Audio_Device.hpp"
#include <Mlib/Audio/OpenALSoft_efx.h>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

AudioDevice::AudioDevice() {
    // select the "preferred device"
    device_ = alcOpenDevice(nullptr);
    if (device_ == nullptr) {
        THROW_OR_ABORT("Could not open audio device");
    }
}

AudioDevice::~AudioDevice() {
    if (!alcCloseDevice(device_)) {
        verbose_abort("Could not close audio device");
    }
}

unsigned int AudioDevice::get_frequency() const {
    ALCint rate;
    alcGetIntegerv(device_, ALC_FREQUENCY, 1, &rate);
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        THROW_OR_ABORT("Could not read audio frequency, code: " + std::to_string(error));
    }
    return integral_cast<unsigned int>(rate);
}

std::string AudioDevice::get_name() const {
    const auto* name = alcGetString(device_, ALC_DEVICE_SPECIFIER);
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        THROW_OR_ABORT("Could not read audio device name, code: " + std::to_string(error));
    }
    if (name == nullptr) {
        THROW_OR_ABORT("Could not read audio device name");
    }
    return name;
}

unsigned int AudioDevice::get_max_auxiliary_sends() const {
    ALCint max_auxiliary_sends = 0;
    alcGetIntegerv(device_, ALC_MAX_AUXILIARY_SENDS, 1, &max_auxiliary_sends);
    if (ALCenum error = alcGetError(device_); error != ALC_NO_ERROR) {
        THROW_OR_ABORT("Could obtain max auxiliary sends per source, code: " + std::to_string(error));
    }
    return max_auxiliary_sends;
}
