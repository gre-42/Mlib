#pragma once
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <memory>

namespace Mlib {

class AudioDevice;

class AudioContext {
    AudioContext(const AudioContext &) = delete;
    AudioContext &operator=(const AudioContext &) = delete;

public:
    AudioContext(AudioDevice &device, unsigned int frequency);
    ~AudioContext();

private:
    DestructionGuards dgs_;
};

}
