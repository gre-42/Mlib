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
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator = (const AudioBuffer&) = delete;
    void load_wave(const std::string& filename);
private:
    ALuint buffer_;
};

}
