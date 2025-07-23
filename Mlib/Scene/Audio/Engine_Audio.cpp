#include "Engine_Audio.hpp"
#include <Mlib/Audio/Audio_Buffer_Sequence_With_Hysteresis.hpp>
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rotating_Frame.hpp>
#include <Mlib/Physics/Units.hpp>
#include <algorithm>

using namespace Mlib;

EngineAudio::EngineAudio(
    const std::string& resource_name,
    std::function<bool()> audio_paused,
    EventEmitter& audio_paused_changed,
    float p_idle,
    float p_reference)
    : cross_fade_{ PositionRequirement::WAITING_FOR_POSITION, std::move(audio_paused), audio_paused_changed }
    , p_reference_{ p_reference }
    , p_idle_{ p_idle }
{
    driving_buffer_sequence_ = AudioResourceContextStack::primary_audio_resources()->get_buffer_sequence(resource_name + ".driving");
    driving_gain_ = AudioResourceContextStack::primary_audio_resources()->get_buffer_sequence_gain(resource_name + ".driving");
}

EngineAudio::~EngineAudio() = default;

void EngineAudio::notify_rotation(
    float engine_angular_velocity,
    float tires_angular_velocity,
    const EnginePowerIntent& engine_power_intent,
    float max_surface_power)
{
    if ((engine_power_intent.state == EngineState::OFF) || (engine_angular_velocity >= 0)) {
        cross_fade_.stop();
    } else {
        // cross_fade_.play(*driving_buffer, driving_gain, std::abs(angular_velocity) / W_MEAN);
        float f = -engine_angular_velocity / rps;
        auto& seq = driving_buffer_sequence_->get_buffer_and_frequency(f);
        float p = engine_power_intent.real_power(
            engine_angular_velocity,
            tires_angular_velocity,
            max_surface_power);
        cross_fade_.play(
            *seq.buffer,
            std::min(1.f, driving_gain_ * std::max(p_idle_, p) / p_reference_),
            f,
            seq.frequency);
    }
}

void EngineAudio::set_location(
    const RotatingFrame<SceneDir, ScenePos, 3>& frame)
{
    cross_fade_.set_position({ frame.location.t, frame.v });
}

void EngineAudio::advance_time(float dt) {
    cross_fade_.advance_time(dt);
}
