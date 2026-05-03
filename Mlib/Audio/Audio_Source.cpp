#include "Audio_Source.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Audio/Audio_Lowpass.hpp>
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Audio/CHK.hpp>
#ifndef USE_PCM_FILTERS
#include <Mlib/Audio/OpenALSoft_efx.h>
#endif
#include <Mlib/Geometry/Primitives/Interval.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

AudioSource::AudioSource(
    PositionRequirement position_requirement,
    float alpha)
    : position_requirement_{ position_requirement }
    , muted_{ false }
    , gain_{ 1.f }
#ifdef __EMSCRIPTEN__
    , loop_{ false }
    , pitch_{ 1.f }
    , position_{ uninitialized, uninitialized }
    , distance_clamping_{ 1.f, INFINITY }
    , last_source_state_{ AL_STOPPED }
#endif
{
    AL_CHK(alGenSources(1, &source_));
    if (position_requirement == PositionRequirement::WAITING_FOR_POSITION) {
        AL_CHK(alSourcef(source_, AL_GAIN, 0.f));
    }
    AudioScene::add_source(*this, alpha);
}

AudioSource::AudioSource(
    const AudioBuffer& buffer,
    PositionRequirement position_requirement,
    float alpha)
    : AudioSource{ position_requirement, alpha }
{
    AL_CHK(alSourcei(source_, AL_BUFFER, integral_cast<ALint>(buffer.handle_)));
    nchannels_ = buffer.nchannels();
}

AudioSource::~AudioSource() {
    AudioScene::remove_source(*this);
    AL_ABORT(alDeleteSources(1, &source_));
}

void AudioSource::set_loop(bool value) {
#ifdef __EMSCRIPTEN__
    loop_ = value;
#else
    AL_CHK(alSourcei(source_, AL_LOOPING, value));
#endif
}

void AudioSource::set_gain(float value) {
    if (std::isnan(value)) {
        throw std::runtime_error("Attempt to set NaN audio gain");
    }
    if (value < 0.f) {
        throw std::runtime_error("Attempt to set negative audio gain");
    }
    if (value > 1.f) {
        throw std::runtime_error("Attempt to set audio gain greater 1");
    }
    if (position_requirement_ != PositionRequirement::WAITING_FOR_POSITION) {
        if (!muted_) {
#ifndef __EMSCRIPTEN__
            AL_CHK(alSourcef(source_, AL_GAIN, value));
#endif
        }
    }
    gain_ = value;
}

void AudioSource::set_pitch(float value) {
    if (std::isnan(value)) {
        throw std::runtime_error("Attempt to set NaN audio pitch");
    }
    if (value < 0.1f) {
        throw std::runtime_error("Attempt to set audio pitch less than 0.1");
    }
    if (value > 5.f) {
        throw std::runtime_error("Attempt to set audio pitch greater 5");
    }
#ifdef __EMSCRIPTEN__
    pitch_ = value;
#else
    AL_CHK(alSourcef(source_, AL_PITCH, value));
#endif
}

void AudioSource::set_position(const AudioSourceState<float>& position) {
    if (nchannels_ != 1) {
        throw std::runtime_error("Attempt to set position of an audio source with #channels != 1");
    }
#ifdef __EMSCRIPTEN__
    position_ = position;
#else
    AL_CHK(alSourcefv(source_, AL_POSITION, (position.position / meters).flat_begin()));
    AL_CHK(alSourcefv(source_, AL_VELOCITY, (position.velocity / (meters / seconds)).flat_begin()));
#endif
    if (position_requirement_ == PositionRequirement::WAITING_FOR_POSITION) {
        if (!muted_) {
#ifndef __EMSCRIPTEN__
            AL_CHK(alSourcef(source_, AL_GAIN, gain_));
#endif
        }
        position_requirement_ = PositionRequirement::POSITION_NOT_REQUIRED;
    }
}

void AudioSource::set_distance_clamping(const Interval<float>& interval) {
#ifdef __EMSCRIPTEN__
    distance_clamping_ = interval;
#else
    AL_CHK(alSourcef(source_, AL_REFERENCE_DISTANCE, interval.min));
    AL_CHK(alSourcef(source_, AL_MAX_DISTANCE, interval.max));
#endif
}

#ifndef USE_PCM_FILTERS
void AudioSource::set_lowpass(const AudioLowpass& lowpass) {
#ifdef __EMSCRIPTEN__
    throw std::runtime_error("Lowpass not supported under Emscripten");
#else
    AL_CHK(alSourcei(source_, AL_DIRECT_FILTER, integral_cast<ALint>(lowpass.handle_)));
#endif
}
#endif

void AudioSource::play() {
#ifdef __EMSCRIPTEN__
    pending_command_ = AL_PLAYING;
#else
    AL_CHK(alSourcePlay(source_));
#endif
}

void AudioSource::pause() {
#ifdef __EMSCRIPTEN__
    pending_command_ = AL_PAUSED;
#else
    AL_CHK(alSourcePause(source_));
#endif
}

void AudioSource::unpause() {
#ifdef __EMSCRIPTEN__
    if (last_source_state_ == AL_PAUSED) {
        pending_command_ = AL_PLAYING;
    }
#else
    ALint source_state;
    AL_CHK(alGetSourcei(source_, AL_SOURCE_STATE, &source_state));
    if (source_state == AL_PAUSED) {
        AL_CHK(alSourcePlay(source_));
    }
#endif
}

void AudioSource::stop() {
#ifdef __EMSCRIPTEN__
    pending_command_ = AL_STOPPED;
#else
    AL_CHK(alSourceStop(source_));
#endif
}

void AudioSource::join() {
    while (true) {
        ALint source_state;
        AL_CHK(alGetSourcei(source_, AL_SOURCE_STATE, &source_state));
        if (source_state != AL_PLAYING) {
            break;
        }
    }
}

void AudioSource::mute() {
    if (!muted_) {
#ifndef __EMSCRIPTEN__
        AL_CHK(alSourcef(source_, AL_GAIN, 0.f));
#endif
        muted_ = true;
    }
}

void AudioSource::unmute() {
    if (muted_) {
#ifndef __EMSCRIPTEN__
        if (position_requirement_ != PositionRequirement::WAITING_FOR_POSITION) {
            AL_CHK(alSourcef(source_, AL_GAIN, gain_));
        }
#endif
        muted_ = false;
    }
}

bool AudioSource::stopped() const {
#ifdef __EMSCRIPTEN__
    return (last_source_state_ == AL_STOPPED);
#else
    ALint source_state;
    AL_CHK(alGetSourcei(source_, AL_SOURCE_STATE, &source_state));
    return (source_state == AL_STOPPED);
#endif
}
