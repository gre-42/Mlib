#include "Audio_Listener.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <mutex>

using namespace Mlib;

bool AudioListener::muted_ = false;
float AudioListener::gain_ = 1.f;
std::optional<AudioListenerState> AudioListener::listener_inverse_state_ = std::nullopt;

void AudioListener::set_transformation(const AudioListenerState& listener_state) {
    listener_inverse_state_ = AudioListenerState{
        .pose = listener_state.pose.inverted(),
        .velocity = -listener_state.pose.irotate(listener_state.velocity)
    };
    // AL_CHK(alListenerfv(AL_POSITION, trafo.t().flat_begin()));
    // float orientation[6] = {
    //     -trafo.R(0, 2),
    //     -trafo.R(1, 2),
    //     -trafo.R(2, 2),
    //     trafo.R(0, 1),
    //     trafo.R(1, 1),
    //     trafo.R(2, 1)
    // };
    // AL_CHK(alListenerfv(AL_ORIENTATION, orientation));
}

std::optional<AudioSourceState<float>> AudioListener::get_relative_position(const AudioSourceState<ScenePos>& state) {
    if (!listener_inverse_state_.has_value()) {
        return std::nullopt;
    }
    const auto& il = *listener_inverse_state_;
    return AudioSourceState<float>{
        .position = il.pose.transform(state.position).casted<float>(),
        .velocity = il.velocity + il.pose.rotate(state.velocity)
    };
}

void AudioListener::set_gain(float f) {
    AL_CHK(alListenerf(AL_GAIN, f));
    gain_ = f;
}

void AudioListener::mute() {
    if (!muted_) {
        AL_CHK(alListenerf(AL_GAIN, 0.f));
        muted_ = true;
    }
}

void AudioListener::unmute() {
    if (muted_) {
        AL_CHK(alListenerf(AL_GAIN, gain_));
        muted_ = false;
    }
}
