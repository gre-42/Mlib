#include "Cross_Fade.hpp"
#include <Mlib/Threads/Set_Thread_Name.hpp>

using namespace Mlib;

void AudioSourceAndGain::apply_gain() {
    source->set_gain(gain_factor * gain);
}

CrossFade::CrossFade(
    PositionRequirement position_requirement,
    std::function<bool()> paused,
    float dgain,
    float dt)
: position_requirement_{position_requirement},
  paused_{std::move(paused)},
  fader_{[this, dgain, dt](){
    set_thread_name("Audio CrossFade");
    while (!fader_.get_stop_token().stop_requested()) {
        {
            bool pause = paused_();
            std::scoped_lock lock{ mutex_ };
            if (pause) {
                for (auto& s : sources_) {
                    s.source->mute();
                }
            } else {
                for (auto& s : sources_) {
                    s.source->unmute();
                }
                float total_gain = 0;
                sources_.remove_if([&](AudioSourceAndGain& sg){
                    if (&sg == &sources_.back()) {
                        sg.gain = std::min(1.f - total_gain, sg.gain + dgain);
                    } else {
                        sg.gain = std::min(1.f - total_gain, sg.gain - dgain);
                        if (sg.gain <= 0) {
                            return true;
                        }
                        total_gain += sg.gain;
                    }
                    sg.apply_gain();
                    return false;
                });
            }
        }
        std::this_thread::sleep_for(std::chrono::duration<float>(dt));
    }}}
{}

CrossFade::~CrossFade()
{}

void CrossFade::play(
    const AudioBuffer& audio_buffer,
    float gain_factor,
    float pitch,
    float buffer_frequency)
{
    auto set_absolute_frequencies = [this, pitch](){
        for (auto& s : sources_) {
            if (!std::isnan(s.buffer_frequency)) {
                s.source->set_pitch(pitch / s.buffer_frequency);
            }
        }
    };
    std::scoped_lock lock{ mutex_ };
    if (!sources_.empty() && (sources_.back().audio_buffer == &audio_buffer)) {
        auto& sg = sources_.back();
        sg.gain_factor = gain_factor;
        sg.apply_gain();
        if (std::isnan(buffer_frequency)) {
            sg.source->set_pitch(pitch);
        } else {
            set_absolute_frequencies();
        }
        return;
    }
    auto& sg = sources_.emplace_back(AudioSourceAndGain{
        .audio_buffer = &audio_buffer,
        .gain = 0.f,
        .gain_factor = gain_factor,
        .buffer_frequency = buffer_frequency,
        .source = std::make_unique<AudioSource>(audio_buffer, position_requirement_)});
    sg.source->set_loop(true);
    sg.apply_gain();
    if (std::isnan(buffer_frequency)) {
        sg.source->set_pitch(pitch);
    } else {
        set_absolute_frequencies();
    }
    sg.source->play();
}

void CrossFade::stop() {
    std::scoped_lock lock{ mutex_ };
    sources_.clear();
}

void CrossFade::set_position(const FixedArray<double, 3>& position) {
    std::scoped_lock lock{ mutex_ };
    for (auto& s : sources_) {
        s.source->set_position(position);
    }
}
