#include "Audio_Device.hpp"
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
