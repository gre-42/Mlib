#pragma once
#include <alc.h>

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
