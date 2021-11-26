#include "Cross_Fade.hpp"

using namespace Mlib;

CrossFade::CrossFade(float dgain, float dt)
: shutdown_requested_{false},
  fader_{[this, dgain, dt](){
    while (!shutdown_requested_) {
        {
            std::lock_guard lock{ mutex_ };
            float total_gain = 0;
            sources_.remove_if([&](std::unique_ptr<AudioSourceAndGain>& sg){
                if (&sg == &sources_.back()) {
                    sg->gain = 1 - total_gain;
                    sg->source.set_gain(sg->gain);
                    return false;
                } else {
                    sg->gain = std::min(1.f - total_gain, sg->gain - dgain);
                    if (sg->gain <= 0) {
                        return true;
                    }
                    total_gain += sg->gain;
                    sg->source.set_gain(sg->gain);
                    return false;
                }
            });
        }
        std::this_thread::sleep_for(std::chrono::duration<float>(dt));
    }}}
{}

CrossFade::~CrossFade() {
    shutdown_requested_ = true;
    fader_.join();
}

void CrossFade::play(const AudioBuffer& audio_buffer, float pitch) {
    std::lock_guard lock{ mutex_ };
    auto sg = std::make_unique<AudioSourceAndGain>();
    sg->source.attach(audio_buffer);
    sg->source.set_gain(0.f);
    sg->source.set_pitch(pitch);
    sg->source.set_loop(true);
    sg->source.play();
    sources_.push_back(std::move(sg));
}

void CrossFade::set_pitch(float value) {
    std::lock_guard lock{ mutex_ };
    if (sources_.empty()) {
        throw std::runtime_error("Cannot set pitch without a source");
    }
    sources_.back()->source.set_pitch(value);
}

void CrossFade::stop() {
    std::lock_guard lock{ mutex_ };
    sources_.clear();
}
