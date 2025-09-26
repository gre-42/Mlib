#include "Lazy_One_Shot_Audio.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/One_Shot_Audio.hpp>

using namespace Mlib;

LazyOneShotAudio::LazyOneShotAudio(
    AudioResources& resources,
    VariableAndHash<std::string> resource_name,
    float alpha)
    : info_{ nullptr }
    , alpha_{ alpha }
    , resources_{ resources }
    , resource_name_{ std::move(resource_name) }
{}

void LazyOneShotAudio::preload() {
    if (buffer_ == nullptr) {
        buffer_ = resources_.get_buffer(resource_name_);
        info_ = &resources_.get_buffer_meta(resource_name_);
    }
}

std::shared_ptr<AudioSourceAndPosition> LazyOneShotAudio::play(
    OneShotAudio& one_shot_audio,
    AudioPeriodicity periodicity,
    const AudioSourceState<ScenePos>& position)
{
    preload();
    return one_shot_audio.play(
        *buffer_,
        info_->lowpass.get(),
        position,
        periodicity,
        info_->distance_clamping,
        info_->gain,
        alpha_);
}
