#pragma once
#include <Mlib/Audio/Audio_Source.hpp>
#include <list>
#include <mutex>
#include <thread>

namespace Mlib {

class AudioBuffer;

struct AudioSourceAndGain;

class CrossFade {
public:
    explicit CrossFade(float dgain = 0.1, float dt = 0.01);
    ~CrossFade();
    void play(
        const AudioBuffer& audio_buffer,
        float gain_factor = 1.f,
        float pitch = 1.f);
    void stop();
    void set_position(const FixedArray<float, 3>& position);
private:
    std::list<std::unique_ptr<AudioSourceAndGain>> sources_;
    std::mutex mutex_;
    std::jthread fader_;
};

}
