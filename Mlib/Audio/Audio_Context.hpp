#pragma once
#include <AL/alc.h>
#include <memory>
#include <string>

namespace Mlib {

class AudioDevice;

class AudioContext {
public:
    explicit AudioContext(AudioDevice& device);
    ~AudioContext();
private:
    ALCcontext* context_;
};

}
