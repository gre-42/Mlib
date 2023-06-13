#pragma once
#include <Mlib/Audio/OpenAL_alc.h>
#include <memory>
#include <string>

namespace Mlib {

class AudioDevice;

class AudioContext {
public:
    explicit AudioContext(AudioDevice& device);
    ~AudioContext();
private:
    ALCdevice* device_;
    ALCcontext* context_;
};

}
