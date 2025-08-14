#include "One_Shot_Audio.hpp"
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Os/Os.hpp>
#include <mutex>

using namespace Mlib;

AudioSourceAndPosition::AudioSourceAndPosition(
    const AudioBuffer& buffer,
    PositionRequirement position_requirement,
    float alpha,
    const AudioSourceState<ScenePos>& position)
    : source{ buffer, position_requirement, alpha }
    , position{ position }
{}

OneShotAudio::OneShotAudio(
    PositionRequirement position_requirement,
    std::function<bool()> paused,
    EventEmitter& paused_changed)
    : position_requirement_{ position_requirement }
    , paused_{ std::move(paused) }
    , erdt_{ paused_changed.insert([this](){ advance_time(); }) }
{}

OneShotAudio::~OneShotAudio() {
    erdt_.reset();
    on_destroy.clear();
}

void OneShotAudio::advance_time(float dt, const StaticWorld& world) {
    advance_time();
}

void OneShotAudio::advance_time() {
    bool pause = paused_();
    std::scoped_lock lock{ mutex_ };
    sources_.remove_if([](AudioSourceAndPosition& sp){
        return sp.source.stopped();
    });
    if (pause) {
        for (auto &sp : sources_) {
            sp.source.pause();
        }
    } else {
        for (auto &sp : sources_) {
            sp.source.unpause();
        }
    }
    for (auto& sp : sources_) {
        AudioScene::set_source_transformation(sp.source, sp.position);
    }
    // if (!pause && !sources_.empty()) {
    //     lraw() << "OneShotAudio::advance_time";
    //     for (auto& sp : sources_) {
    //         lraw() << &sp.source;
    //     }
    //     AudioScene::print(lraw().ref());
    // }
}

void OneShotAudio::play(
    const AudioBuffer& audio_buffer,
    const AudioSourceState<ScenePos>& position,
    float gain,
    float alpha)
{
    std::scoped_lock lock{ mutex_ };
    auto& sp = sources_.emplace_back(
        audio_buffer,
        position_requirement_,
        alpha,
        position);
    AudioScene::set_source_transformation(sp.source, sp.position);
    sp.source.set_loop(false);
    sp.source.set_gain(gain);
    sp.source.play();
}

void OneShotAudio::stop() {
    std::scoped_lock lock{ mutex_ };
    sources_.clear();
}
