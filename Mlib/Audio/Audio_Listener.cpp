#include "Audio_Listener.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

bool AudioListener::muted_ = false;
float AudioListener::gain_ = 1.f;
std::optional<TransformationMatrix<float, double, 3>> AudioListener::view_matrix_ = std::nullopt;

void AudioListener::set_transformation(const TransformationMatrix<float, double, 3>& trafo) {
    view_matrix_ = trafo.inverted();
    // AL_CHK(alListenerfv(AL_POSITION, trafo.t().flat_begin()));
    // float orientation[6] = {
    //     -trafo.R()(0u, 2u),
    //     -trafo.R()(1u, 2u),
    //     -trafo.R()(2u, 2u),
    //     trafo.R()(0u, 1u),
    //     trafo.R()(1u, 1u),
    //     trafo.R()(2u, 1u)
    // };
    // AL_CHK(alListenerfv(AL_ORIENTATION, orientation));
}

std::optional<FixedArray<float, 3>> AudioListener::get_relative_position(const FixedArray<double, 3>& position) {
    if (!view_matrix_.has_value()) {
        return std::nullopt;
    }
    return view_matrix_.value().transform(position).casted<float>();
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
