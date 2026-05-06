#include "Audio_Listener.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Testing/Assert_Range.hpp>
#include <mutex>

using namespace Mlib;

FastMutex AudioListener::mutex_;
bool AudioListener::muted_ = false;
float AudioListener::gain_ = 1.f;
std::optional<AudioListenerState> AudioListener::listener_inverse_state_ = std::nullopt;

void AudioListener::set_transformation(const AudioListenerState& listener_state) {
    std::scoped_lock lock{mutex_};
    listener_inverse_state_ = AudioListenerState{
        .pose = listener_state.pose.inverted(),
        .velocity = -listener_state.pose.irotate(listener_state.velocity)
    };
    // AL_CHK(alListenerfv(AL_POSITION, assert_finite(trafo.t(), "Audio position").flat_begin()));
    // float orientation[6] = {
    //     -trafo.R(0, 2),
    //     -trafo.R(1, 2),
    //     -trafo.R(2, 2),
    //     trafo.R(0, 1),
    //     trafo.R(1, 1),
    //     trafo.R(2, 1)
    // };
    // AL_CHK(alListenerfv(AL_ORIENTATION, assert_finite(orientation, "Audio orientation")));
}

std::optional<AudioSourceState<float>> AudioListener::get_relative_position(const AudioSourceState<ScenePos>& state) {
    std::scoped_lock lock{mutex_};
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
    std::scoped_lock lock{mutex_};
    assert_finite(f, "Audio listener gain (set_gain)");
    if (!muted_) {
        AL_CHK(alListenerf(AL_GAIN, f));
    }
    gain_ = f;
}

void AudioListener::mute() {
    std::scoped_lock lock{mutex_};
    if (!muted_) {
        AL_CHK(alListenerf(AL_GAIN, 0.f));
        muted_ = true;
    }
}

void AudioListener::unmute() {
    std::scoped_lock lock{mutex_};
    if (muted_) {
        AL_CHK(alListenerf(AL_GAIN, assert_finite(gain_, "Audio listener gain (unmute)")));
        muted_ = false;
    }
}
