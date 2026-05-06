#ifndef USE_PCM_FILTERS
#include "Audio_Equalizer.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/OpenALSoft_efx.h>
#include <Mlib/Testing/Assert_Range.hpp>

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
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_LOW_GAIN, assert_range(parameters.low_gain, 0.f, 1.f, "Equalizer low gain")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_LOW_CUTOFF, assert_range(parameters.low_cutoff, 0.f, 1.f, "Equalizer low cutoff")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_GAIN, assert_range(parameters.mid1_gain, 0.f, 1.f, "Equalizer mid1 gain")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_CENTER, assert_range(parameters.mid1_center, 0.f, 1.f, "Equalizer mid1 center")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID1_WIDTH, assert_range(parameters.mid1_width, 0.f, 1.f, "Equalizer mid1 width")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_GAIN, assert_range(parameters.mid2_gain, 0.f, 1.f, "Equalizer mid2 gain")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_CENTER, assert_range(parameters.mid2_center, 0.f, 1.f, "Equalizer mid2 center")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_MID2_WIDTH, assert_range(parameters.mid2_width, 0.f, 1.f, "Equalizer mid2 width")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_HIGH_GAIN, assert_range(parameters.high_gain, 0.f, 1.f, "Equalizer high gain")));
    AL_CHK(alEffectf(res->handle_, AL_EQUALIZER_HIGH_CUTOFF, assert_range(parameters.high_cutoff, 0.f, 1.f, "Equalizer high cutoff")));
    return res;
}
#endif
