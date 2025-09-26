#include "Audio_Equalizer.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/OpenALSoft_efx.h>

using namespace Mlib;

AudioEqualizer::AudioEqualizer()
{
    AL_CHK(alGenEffects(1, &handle_));
}

AudioEqualizer::~AudioEqualizer() {
    AL_ABORT(alDeleteEffects((ALuint)1, &handle_));
}

std::shared_ptr<AudioEqualizer> AudioEqualizer::create(const AudioEqualizerInformation& parameters) {
    auto res = std::make_shared<AudioEqualizer>();
    AL_CHK(alEffecti(res->handle_, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_LOW_GAIN, parameters.low_gain));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_LOW_CUTOFF, parameters.low_cutoff));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_GAIN, parameters.mid1_gain));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_CENTER, parameters.mid1_center));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_WIDTH, parameters.mid1_width));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_GAIN, parameters.mid2_gain));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_CENTER, parameters.mid2_center));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_WIDTH, parameters.mid2_width));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_HIGH_GAIN, parameters.high_gain));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_HIGH_CUTOFF, parameters.high_cutoff));
    return res;
}
