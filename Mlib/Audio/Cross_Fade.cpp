#include "Cross_Fade.hpp"
#include <Mlib/Threads/Set_Thread_Name.hpp>

using namespace Mlib;

namespace Mlib {

struct AudioSourceAndGain {
    const AudioBuffer* audio_buffer;
    float gain;
    float gain_factor;
    AudioSource source;
    void apply_gain();
};

}

void AudioSourceAndGain::apply_gain() {
    source.set_gain(gain_factor * gain);
}

CrossFade::CrossFade(
    const std::function<bool()>& paused,
    float dgain,
    float dt)
: paused_{paused},
  fader_{[this, dgain, dt](){
    set_thread_name("Audio CrossFade");
    while (!fader_.get_stop_token().stop_requested()) {
        {
            std::lock_guard lock{ mutex_ };
            if (paused_()) {
                for (auto& s : sources_) {
                    s->source.mute();
                }
            } else {
                for (auto& s : sources_) {
                    s->source.unmute();
                }
                float total_gain = 0;
                sources_.remove_if([&](std::unique_ptr<AudioSourceAndGain>& sg){
                    if (&sg == &sources_.back()) {
                        sg->gain = 1 - total_gain;
                    } else {
                        sg->gain = std::min(1.f - total_gain, sg->gain - dgain);
                        if (sg->gain <= 0) {
                            return true;
                        }
                        total_gain += sg->gain;
                    }
                    sg->apply_gain();
                    return false;
                });
            }
        }
        std::this_thread::sleep_for(std::chrono::duration<float>(dt));
    }}}
{}

CrossFade::~CrossFade()
{}

void CrossFade::play(const AudioBuffer& audio_buffer, float gain_factor, float pitch) {
    std::lock_guard lock{ mutex_ };
    if (!sources_.empty() && (sources_.back()->audio_buffer == &audio_buffer)) {
        sources_.back()->gain_factor = gain_factor;
        sources_.back()->apply_gain();
        sources_.back()->source.set_pitch(pitch);
        return;
    }
    auto sg = std::make_unique<AudioSourceAndGain>();
    sg->audio_buffer = &audio_buffer;
    sg->gain = 0.f;
    sg->gain_factor = gain_factor;
    sg->source.attach(audio_buffer);
    sg->source.set_pitch(pitch);
    sg->source.set_loop(true);
    sg->apply_gain();
    sg->source.play();
    sources_.push_back(std::move(sg));
}

void CrossFade::stop() {
    std::lock_guard lock{ mutex_ };
    sources_.clear();
}

void CrossFade::set_position(const FixedArray<float, 3>& position) {
    std::lock_guard lock{ mutex_ };
    for (auto& s : sources_) {
        s->source.set_position(position);
    }
}
