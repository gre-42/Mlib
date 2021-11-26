#pragma once
#include <AL/al.h>

namespace Mlib {

class AudioBuffer;

class AudioSource {
public:
    AudioSource();
    ~AudioSource();
    AudioSource(const AudioSource&) = delete;
    AudioSource& operator = (const AudioSource&) = delete;
    void attach(const AudioBuffer& buffer);
    void set_loop(bool value);
    void set_gain(float f);
    void set_pitch(float f);
    void play();
    void join();
private:
    ALuint source_;
};

}
