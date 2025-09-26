#include "Audio_Lowpass.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/OpenALSoft_efx.h>

using namespace Mlib;

AudioLowpass::AudioLowpass()
{
    AL_CHK(alGenFilters(1, &handle_));
}

AudioLowpass::~AudioLowpass() {
    AL_ABORT(alDeleteFilters((ALuint)1, &handle_));
}

std::shared_ptr<AudioLowpass> AudioLowpass::create(const AudioLowpassInformation& parameters) {
    auto res = std::make_shared<AudioLowpass>();
    AL_CHK(alFilteri(res->handle_, AL_FILTER_TYPE, AL_FILTER_LOWPASS));
    AL_CHK(alFilterf(res->handle_, AL_LOWPASS_GAIN, parameters.gain));
    AL_CHK(alFilterf(res->handle_, AL_LOWPASS_GAINHF, parameters.gain_hf));
    return res;
}
