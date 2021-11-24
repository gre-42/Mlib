#include "Audio_Device.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdexcept>

using namespace Mlib;

AudioDevice::AudioDevice() {
    // select the "preferred device"
    device_ = alcOpenDevice(nullptr);
    if (device_ == nullptr) {
        throw std::runtime_error("Could not open audio device");
    }
}

AudioDevice::~AudioDevice() {
    alcCloseDevice(device_);
}
