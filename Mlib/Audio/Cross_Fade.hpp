#pragma once
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <list>

namespace Mlib {

class AudioBuffer;
template <class TPosition>
struct AudioSourceState;

using Gain = FixedPointNumber<int32_t, (1 << 13)>;

struct AudioSourceAndGain {
    const AudioBuffer* audio_buffer;
    Gain gain;
    float gain_factor;
    float buffer_frequency;
    std::unique_ptr<AudioSource> source;
    void apply_gain();
};

class CrossFade {
    CrossFade(const CrossFade &) = delete;
    CrossFade &operator=(const CrossFade &) = delete;
public:
    explicit CrossFade(PositionRequirement position_requirement,
                       std::function<bool()> paused,
                       float dgain = 0.02f,
                       float dt = 0.01f);
    ~CrossFade();
    void play(const AudioBuffer &audio_buffer,
              float gain_factor = 1.f,
              float pitch = 1.f,
              float buffer_frequency = NAN,
              float alpha = NAN);
    void stop();
    void set_position(const AudioSourceState<ScenePos> &position);
    void print(std::ostream& ostr) const;

private:
    void update_gain_unsafe(float dgain);
    void update_pitch_unsafe(float pitch);
    void print_unsafe(std::ostream& ostr) const;

    PositionRequirement position_requirement_;
    Gain total_gain_;
    std::list<AudioSourceAndGain> sources_;
    mutable FastMutex mutex_;
    std::function<bool()> paused_;
    JThread fader_;
};

}
