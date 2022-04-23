#include "Engine_Audio.hpp"
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Physics/Units.hpp>
#endif

static const float WHEEL_RADIUS = 0.25f;

using namespace Mlib;

EngineAudio::EngineAudio(
    const std::string& resource_name,
    const std::atomic_bool& paused)
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

void EngineAudio::notify_off() {
#ifndef WITHOUT_ALUT
    cross_fade_.stop();
#endif
}

void EngineAudio::notify_idle(float w) {
#ifndef WITHOUT_ALUT
    if (std::abs(w * WHEEL_RADIUS) * s < 1) {
        cross_fade_.play(*idle_buffer, idle_gain);
    } else {
        cross_fade_.play(*driving_buffer, driving_gain, std::abs(w * WHEEL_RADIUS) / (80.f / 3.6f) * s);
    }
#endif
}

void EngineAudio::notify_driving(float w) {
#ifndef WITHOUT_ALUT
    cross_fade_.play(*driving_buffer, driving_gain, std::abs(w * WHEEL_RADIUS) / (80.f / 3.6f) * s);
#endif
}

void EngineAudio::set_position(const FixedArray<float, 3>& position) {
#ifndef WITHOUT_ALUT
    cross_fade_.set_position(position);
#endif
}
