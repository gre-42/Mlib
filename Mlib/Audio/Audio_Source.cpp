#include "Audio_Source.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

AudioSource::AudioSource(PositionRequirement position_requirement)
: position_requirement_{position_requirement},
  muted_{false},
  gain_{1.f}
{
    AL_CHK(alGenSources(1, &source_));
    if (position_requirement == PositionRequirement::WAITING_FOR_POSITION) {
        AL_CHK(alSourcef(source_, AL_GAIN, 0.f));
    }
}

AudioSource::AudioSource(
    const AudioBuffer& buffer,
    PositionRequirement position_requirement)
: AudioSource{position_requirement}
{
    if (!buffer.buffer_.has_value()) {
        THROW_OR_ABORT("Cannot attach null audio buffer");
    }
    AL_CHK(alSourcei(source_, AL_BUFFER, (ALint)buffer.buffer_.value()));
}

AudioSource::~AudioSource() {
    AL_ABORT(alDeleteSources(1, &source_));
}

void AudioSource::set_loop(bool value) {
    AL_CHK(alSourcei(source_, AL_LOOPING, value));
}

void AudioSource::set_gain(float value) {
    if (position_requirement_ != PositionRequirement::WAITING_FOR_POSITION) {
        if (!muted_) {
            AL_CHK(alSourcef(source_, AL_GAIN, value));
        }
    }
    gain_ = value;
}

void AudioSource::set_pitch(float value) {
    AL_CHK(alSourcef(source_, AL_PITCH, value));
}

void AudioSource::set_position(const AudioSourceState<double>& position) {
    auto relpos = AudioListener::get_relative_position(position);
    if (!relpos.has_value()) {
        return;
    }
    AL_CHK(alSourcefv(source_, AL_POSITION, (relpos.value().position / meters).flat_begin()));
    AL_CHK(alSourcefv(source_, AL_VELOCITY, (relpos.value().velocity / (meters / s)).flat_begin()));
    if (position_requirement_ == PositionRequirement::WAITING_FOR_POSITION) {
        if (!muted_) {
            AL_CHK(alSourcef(source_, AL_GAIN, gain_));
        }
        position_requirement_ = PositionRequirement::POSITION_NOT_REQUIRED;
    }
}

void AudioSource::play() {
    AL_CHK(alSourcePlay(source_));
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
        AL_CHK(alSourcef(source_, AL_GAIN, 0.f));
        muted_ = true;
    }
}

void AudioSource::unmute() {
    if (muted_) {
        if (position_requirement_ != PositionRequirement::WAITING_FOR_POSITION) {
            AL_CHK(alSourcef(source_, AL_GAIN, gain_));
        }
        muted_ = false;
    }
}
