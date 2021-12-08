#include "Audio_Listener.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

void AudioListener::set_transformation(const TransformationMatrix<float, 3>& trafo) {
    AL_CHK(alListenerfv(AL_POSITION, trafo.t().flat_begin()));
    float orientation[6] = {
        -trafo.R()(0, 2),
        -trafo.R()(1, 2),
        -trafo.R()(2, 2),
        trafo.R()(0, 1),
        trafo.R()(1, 1),
        trafo.R()(2, 1)
    };
    AL_CHK(alListenerfv(AL_ORIENTATION, orientation));
}

void AudioListener::set_gain(float f) {
    AL_CHK(alListenerf(AL_GAIN, f));
}
