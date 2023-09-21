#include "Audio_Device.hpp"
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
