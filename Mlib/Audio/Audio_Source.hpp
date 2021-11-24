#pragma once
#include <AL/al.h>

namespace Mlib {

class AudioBuffer;

class AudioSource {
public:
    AudioSource();
    ~AudioSource();
    void attach(AudioBuffer& buffer);
    void play();
    void join();
private:
    ALuint source_;
};

}
