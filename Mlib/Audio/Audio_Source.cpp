#include "Audio_Source.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Audio/Audio_Buffer.hpp>
#include <Mlib/Audio/CHK.hpp>

using namespace Mlib;

AudioSource::AudioSource()
: muted_{false},
  gain_{1.f}
{
    AL_CHK(alGenSources(1, &source_));
}

AudioSource::~AudioSource() {
    AL_WARN(alDeleteSources(1, &source_));
}

void AudioSource::attach(const AudioBuffer& buffer) {
    AL_CHK(alSourcei(source_, AL_BUFFER, buffer.buffer_));
}    

void AudioSource::set_loop(bool value) {
    AL_CHK(alSourcei(source_, AL_LOOPING, value));
}

void AudioSource::set_gain(float value) {
    AL_CHK(alSourcef(source_, AL_GAIN, value));
    gain_ = value;
}

void AudioSource::set_pitch(float value) {
    AL_CHK(alSourcef(source_, AL_PITCH, value));
}

void AudioSource::set_position(const FixedArray<float, 3>& position) {
    AL_CHK(alSourcefv(source_, AL_POSITION, position.flat_begin()));
}

void AudioSource::play() {
    AL_CHK(alSourcePlay(source_));
}

void AudioSource::join() {
    ALint source_state;
    AL_CHK(alGetSourcei(source_, AL_SOURCE_STATE, &source_state));
    while (source_state == AL_PLAYING) {
        AL_CHK(alGetSourcei(source_, AL_SOURCE_STATE, &source_state));
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
        AL_CHK(alSourcef(source_, AL_GAIN, gain_));
        muted_ = false;
    }
}
