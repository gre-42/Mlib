#pragma once
#include <al.h>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
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
    void set_position(const FixedArray<float, 3>& position);
    void play();
    void join();
    void mute();
    void unmute();
private:
    ALuint source_;
    bool muted_;
    float gain_;
};

}
