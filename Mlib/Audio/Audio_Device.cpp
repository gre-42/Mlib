#include "Audio_Device.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <al.h>
#include <alc.h>
#include <stdexcept>

using namespace Mlib;

AudioDevice::AudioDevice() {
    // select the "preferred device"
    std::lock_guard mutex{ Mlib::al_error_mutex };
    device_ = alcOpenDevice(nullptr);
    if (device_ == nullptr) {
        throw std::runtime_error("Could not open audio device");
    }
}

AudioDevice::~AudioDevice() {
    std::lock_guard mutex{ Mlib::al_error_mutex };
    if (!alcCloseDevice(device_)) {
        std::cerr << "Could not close audio device" << std::endl;
    }
}
