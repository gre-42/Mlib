#include "One_Shot_Audio.hpp"
#include <Mlib/Audio/Audio_Periodicity.hpp>
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Geometry/Intersection/Interval.hpp>
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

AudioSourceAndPosition::~AudioSourceAndPosition() = default;

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
    sources_.remove_if([](auto& sp){
        return sp->source.stopped();
    });
    if (pause) {
        for (auto &sp : sources_) {
            sp->source.pause();
        }
    } else {
        for (auto &sp : sources_) {
            sp->source.unpause();
        }
    }
    for (auto& sp : sources_) {
        AudioScene::set_source_transformation(sp->source, sp->position);
    }
    // if (!pause && !sources_.empty()) {
    //     lraw() << "OneShotAudio::advance_time";
    //     for (auto& sp : sources_) {
    //         lraw() << &sp.source;
    //     }
    //     AudioScene::print(lraw().ref());
    // }
}

std::shared_ptr<AudioSourceAndPosition> OneShotAudio::play(
    const AudioBuffer& audio_buffer,
    const AudioLowpass* lowpass,
    const AudioSourceState<ScenePos>& position,
    AudioPeriodicity periodicity,
    const std::optional<Interval<float>>& distance_clamping,
    float gain,
    float alpha)
{
    std::scoped_lock lock{ mutex_ };
    auto sp = sources_.emplace_back(std::make_shared<AudioSourceAndPosition>(
        audio_buffer,
        position_requirement_,
        alpha,
        position));
    AudioScene::set_source_transformation(sp->source, sp->position);
    sp->source.set_loop(periodicity == AudioPeriodicity::PERIODIC);
    sp->source.set_gain(gain);
    if (distance_clamping.has_value()) {
        sp->source.set_distance_clamping(*distance_clamping);
    }
    if (lowpass != nullptr) {
        sp->source.set_lowpass(*lowpass);
    }
    sp->source.play();
    return sp;
}

void OneShotAudio::stop() {
    std::scoped_lock lock{ mutex_ };
    sources_.clear();
}
