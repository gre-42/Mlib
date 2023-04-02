#pragma once
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <cmath>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class AudioBuffer;

struct AudioSourceAndGain {
    const AudioBuffer* audio_buffer;
    float gain;
    float gain_factor;
    float buffer_frequency;
    std::unique_ptr<AudioSource> source;
    void apply_gain();
};

class CrossFade {
public:
    explicit CrossFade(
        std::function<bool()> paused,
        float dgain = 0.02f,
        float dt = 0.01f);
    ~CrossFade();
    void play(
        const AudioBuffer& audio_buffer,
        float gain_factor = 1.f,
        float pitch = 1.f,
        float buffer_frequency = NAN);
    void stop();
    void set_position(const FixedArray<float, 3>& position);
private:
    std::list<AudioSourceAndGain> sources_;
    std::mutex mutex_;
    std::function<bool()> paused_;
    JThread fader_;
};

}
