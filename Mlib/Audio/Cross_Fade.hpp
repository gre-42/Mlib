#pragma once
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

class AudioBuffer;

struct AudioSourceAndGain;

class CrossFade {
public:
    explicit CrossFade(
        const std::function<bool()>& paused,
        float dgain = 0.1f,
        float dt = 0.01f);
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
    const std::function<bool()>& paused_;
    JThread fader_;
};

}
