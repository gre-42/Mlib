#pragma once
#include <AL/al.h>
#include <string>

namespace Mlib {

class AudioSource;

class AudioBuffer {
    friend AudioSource;
public:
    AudioBuffer();
    ~AudioBuffer();
    void load_wave(const std::string& filename);
private:
    ALuint buffer_;
};

}
