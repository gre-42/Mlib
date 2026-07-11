#pragma once
#include <Mlib/Audio/OpenAL_alc.h>
#include <string>

namespace Mlib {

class AudioContext;

class AudioDevice {
    friend AudioContext;
    AudioDevice(const AudioDevice &) = delete;
    AudioDevice &operator=(const AudioDevice &) = delete;

public:
    explicit AudioDevice();
    ~AudioDevice();

    static unsigned int get_frequency();
    static  std::string get_name();
#ifndef USE_PCM_FILTERS
    static unsigned int get_max_auxiliary_sends();
#endif

private:
    static ALCdevice *device_;
};

}
