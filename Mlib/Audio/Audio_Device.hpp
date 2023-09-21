#pragma once
#include <Mlib/Audio/OpenAL_alc.h>

namespace Mlib {

class AudioContext;

class AudioDevice {
    friend AudioContext;
    AudioDevice(const AudioDevice &) = delete;
    AudioDevice &operator=(const AudioDevice &) = delete;

public:
    explicit AudioDevice();
    ~AudioDevice();

    unsigned int get_frequency() const;

private:
    ALCdevice *device_;
};

}
