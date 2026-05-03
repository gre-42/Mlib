#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <cstddef>
#include <cstdint>
#ifdef __EMSCRIPTEN__
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Geometry/Primitives/Interval.hpp>
#endif

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class AudioBuffer;
class AudioLowpass;
template <class TPosition>
struct AudioSourceState;
template <class T>
struct Interval;
class AudioScene;

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
    friend class AudioScene;
public:
    AudioSource(
        const AudioBuffer& buffer,
        PositionRequirement position_requirement,
        float alpha = 1.f);
    ~AudioSource();
    void set_loop(bool value);
    void set_gain(float f);
    void set_pitch(float f);
    void set_position(const AudioSourceState<float>& position);
    void set_distance_clamping(const Interval<float>& interval);
#ifndef USE_PCM_FILTERS
    void set_lowpass(const AudioLowpass& lowpass);
#endif
    void play();
    void pause();
    void unpause();
    void stop();
    void join();
    void mute();
    void unmute();
    bool stopped() const;
private:
    uint32_t nchannels_;
    ALuint source_;
    PositionRequirement position_requirement_;
    bool muted_;
    float gain_;
#ifdef __EMSCRIPTEN__
    bool loop_;
    float pitch_;
    AudioSourceState<float> position_;
    Interval<float> distance_clamping_;
    ALint last_source_state_;
    std::optional<ALint> pending_command_;
#endif
};

}
