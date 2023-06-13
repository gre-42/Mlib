#pragma once
#include <Mlib/Audio/OpenAL_alc.h>

namespace Mlib {

class AudioContext;

class AudioDevice {
    friend AudioContext;
public:
    explicit AudioDevice();
    ~AudioDevice();
private:
    ALCdevice* device_;
};

}
