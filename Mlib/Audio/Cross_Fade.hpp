#pragma once
#include <Mlib/Audio/Audio_Source.hpp>
#include <list>
#include <mutex>
#include <thread>

namespace Mlib {

class AudioBuffer;

struct AudioSourceAndGain {
    AudioSource source;
    float gain;
};

class CrossFade {
public:
    explicit CrossFade(float dgain = 0.1, float dt = 0.01);
    ~CrossFade();
    void play(const AudioBuffer& audio_buffer, float pitch = 1.f);
    void set_pitch(float value);
    void stop();
private:
    std::list<std::unique_ptr<AudioSourceAndGain>> sources_;
    bool shutdown_requested_;
    std::mutex mutex_;
    std::thread fader_;
};

}
