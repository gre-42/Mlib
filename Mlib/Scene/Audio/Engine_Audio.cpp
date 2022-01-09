#include "Engine_Audio.hpp"
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>

static const float WHEEL_RADIUS = 0.25f;

using namespace Mlib;

EngineAudio::EngineAudio(const std::string& resource_name) {
    idle_buffer = AudioResourceContextStack::primary_audio_resources()->get_buffer(resource_name + ".idle");
    driving_buffer = AudioResourceContextStack::primary_audio_resources()->get_buffer(resource_name + ".driving");
    idle_gain = AudioResourceContextStack::primary_audio_resources()->get_gain(resource_name + ".idle");
    driving_gain = AudioResourceContextStack::primary_audio_resources()->get_gain(resource_name + ".driving");
}

void EngineAudio::notify_off() {
    cross_fade_.stop();
}

void EngineAudio::notify_idle(float w) {
    if (std::abs(w * WHEEL_RADIUS) < 1) {
        cross_fade_.play(*idle_buffer, idle_gain);
    } else {
        cross_fade_.play(*driving_buffer, driving_gain, std::abs(w * WHEEL_RADIUS) / (80.f / 3.6f));
    }
}

void EngineAudio::notify_driving(float w) {
    cross_fade_.play(*driving_buffer, driving_gain, std::abs(w * WHEEL_RADIUS) / (80.f / 3.6f));
}

void EngineAudio::set_position(const FixedArray<float, 3>& position) {
    cross_fade_.set_position(position);
}
