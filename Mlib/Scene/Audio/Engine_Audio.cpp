#include "Engine_Audio.hpp"
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>

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

void EngineAudio::notify_idle() {
    cross_fade_.play(*idle_buffer, idle_gain);
}

void EngineAudio::notify_driving(float w) {
    cross_fade_.play(*driving_buffer, driving_gain, std::abs(w * 0.25f) / (200.f / 3.6f));
}
