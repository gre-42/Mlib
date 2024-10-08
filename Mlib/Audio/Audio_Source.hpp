#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class AudioBuffer;
template <class TPosition>
struct AudioSourceState;

enum class PositionRequirement {
    WAITING_FOR_POSITION,
    POSITION_NOT_REQUIRED
};

class AudioSource {
    AudioSource(const AudioSource &) = delete;
    AudioSource &operator=(const AudioSource &) = delete;
    explicit AudioSource(
        PositionRequirement position_requirement,
        float alpha);

public:
    AudioSource(
        const AudioBuffer &buffer,
        PositionRequirement position_requirement,
        float alpha = 1.f);
    ~AudioSource();
    void set_loop(bool value);
    void set_gain(float f);
    void set_pitch(float f);
    void set_position(const AudioSourceState<float> &position);
    void play();
    void join();
    void mute();
    void unmute();

private:
    ALuint source_;
    PositionRequirement position_requirement_;
    bool muted_;
    float gain_;
};

}
