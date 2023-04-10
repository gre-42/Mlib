#include "Engine_Audio.hpp"
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Buffer_Sequence.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Units.hpp>
#endif

using namespace Mlib;

EngineAudio::EngineAudio(
    const std::string& resource_name,
    const std::function<bool()>& paused,
    float p_reference,
    float p_idle)
#ifndef WITHOUT_ALUT
: cross_fade_{ PositionRequirement::WAITING_FOR_POSITION, paused },
  p_reference_{p_reference},
  p_idle_{p_idle}
{
    driving_buffer_sequence_ = AudioResourceContextStack::primary_audio_resources()->get_buffer_sequence(resource_name + ".driving");
    driving_gain_ = AudioResourceContextStack::primary_audio_resources()->get_buffer_sequence_gain(resource_name + ".driving");
}
#else
{}
#endif

EngineAudio::~EngineAudio() = default;

void EngineAudio::notify_rotation(
    float engine_angular_velocity,
    float tires_angular_velocity,
    const EnginePowerIntent& engine_power_intent,
    float max_surface_power)
{
#ifndef WITHOUT_ALUT
    if ((engine_power_intent.state == EngineState::OFF) || (engine_angular_velocity >= 0)) {
        cross_fade_.stop();
    } else {
        // cross_fade_.play(*driving_buffer, driving_gain, std::abs(angular_velocity) / W_MEAN);
        float f = -engine_angular_velocity / rps;
        auto& seq = driving_buffer_sequence_->get_buffer_and_frequency(f);
        float p =
            std::isnan(engine_power_intent.surface_power) ||
            std::isnan(tires_angular_velocity) ||
            (sign(engine_power_intent.surface_power) == sign(tires_angular_velocity))
                ? 0.f
                : std::clamp(
                    std::abs(engine_power_intent.surface_power),
                    0.f,
                    max_surface_power) * engine_power_intent.drive_relaxation;
        cross_fade_.play(
            *seq.buffer,
            driving_gain_ * std::max(p_idle_, p) / p_reference_,
            f,
            seq.frequency);
    }
#endif
}

void EngineAudio::set_position(const FixedArray<double, 3>& position) {
#ifndef WITHOUT_ALUT
    cross_fade_.set_position(position);
#endif
}
