#include "Engine_Audio.hpp"
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Units.hpp>
#endif

using namespace Mlib;

#ifndef WITHOUT_ALUT
static const float W_MEAN = 2000.f * rpm;
static const float W_IDLE = 100.f * rpm;
#endif

EngineAudio::EngineAudio(
    const std::string& resource_name,
    const std::function<bool()>& paused)
#ifndef WITHOUT_ALUT
: cross_fade_{ paused }
{
    idle_buffer = AudioResourceContextStack::primary_audio_resources()->get_buffer(resource_name + ".idle");
    driving_buffer = AudioResourceContextStack::primary_audio_resources()->get_buffer(resource_name + ".driving");
    idle_gain = AudioResourceContextStack::primary_audio_resources()->get_gain(resource_name + ".idle");
    driving_gain = AudioResourceContextStack::primary_audio_resources()->get_gain(resource_name + ".driving");
}
#else
{}
#endif

void EngineAudio::notify_rotation(
    float angular_velocity,
    const EnginePowerIntent& engine_power_intent)
{
#ifndef WITHOUT_ALUT
    if (engine_power_intent.state == EngineState::OFF) {
        cross_fade_.stop();
    } else if (std::abs(angular_velocity) < W_IDLE) {
        cross_fade_.play(*idle_buffer, idle_gain);
    } else {
        cross_fade_.play(*driving_buffer, driving_gain, std::abs(angular_velocity) / W_MEAN);
    }
#endif
}

void EngineAudio::set_position(const FixedArray<float, 3>& position) {
#ifndef WITHOUT_ALUT
    cross_fade_.set_position(position);
#endif
}
