#include "Lazy_One_Shot_Audio.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/One_Shot_Audio.hpp>

using namespace Mlib;

LazyOneShotAudio::LazyOneShotAudio(
    AudioResources& resources,
    std::string resource_name,
    float alpha)
    : gain_{ NAN }
    , alpha_{ alpha }
    , resources_{ resources }
    , resource_name_{ std::move(resource_name) }
{}

void LazyOneShotAudio::preload() {
    if (buffer_ == nullptr) {
        buffer_ = resources_.get_buffer(resource_name_);
        gain_ = resources_.get_buffer_gain(resource_name_);
    }
}

void LazyOneShotAudio::play(
    OneShotAudio& one_shot_audio,
    const AudioSourceState<ScenePos>& position)
{
    preload();
    one_shot_audio.play(*buffer_, position, gain_, alpha_);
}
