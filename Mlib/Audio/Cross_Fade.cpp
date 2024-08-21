#include "Cross_Fade.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <mutex>

using namespace Mlib;

static const float EPS = 1e-4f;

void AudioSourceAndGain::apply_gain() {
    source->set_gain(gain_factor * gain);
}

CrossFade::CrossFade(
    PositionRequirement position_requirement,
    std::function<bool()> paused,
    float dgain,
    float dt)
    : position_requirement_{ position_requirement }
    , total_gain_{ 0.f }
    , paused_{ std::move(paused) }
    , fader_{ [this, dgain, dt]() {
        try {
            ThreadInitializer ti{"Audio CrossFade", ThreadAffinity::POOL};
            while (!fader_.get_stop_token().stop_requested()) {
                {
                    bool pause = paused_();
                    std::scoped_lock lock{ mutex_ };
                    if (pause) {
                        for (auto &s : sources_) {
                            s.source->mute();
                        }
                    } else {
                        for (auto &s : sources_) {
                            s.source->unmute();
                        }
                        update_gain_unsafe(dgain);
                    }
                }
                Mlib::sleep_for(std::chrono::duration<float>(dt));
            }
        } catch (const std::exception& e) {
            verbose_abort("Exception in cross-fade: " + std::string(e.what()));
        }
    }}
{}

CrossFade::~CrossFade()
{}

// From: https://stackoverflow.com/questions/14579957/std-container-c-move-to-front
template <class T>
void move_to_end(std::list<T>& list, typename std::list<T>::iterator it) {
    list.splice(list.end(), list, it);
}

void CrossFade::play(
    const AudioBuffer& audio_buffer,
    float gain_factor,
    float pitch,
    float buffer_frequency)
{
    std::scoped_lock lock{ mutex_ };
    auto sg_it = std::find_if(
        sources_.begin(),
        sources_.end(),
        [&audio_buffer](const auto& s){ return s.audio_buffer == &audio_buffer; });
    if (sg_it != sources_.end()) {
        move_to_end(sources_, sg_it);
        auto& sg = *sg_it;
        sg.gain_factor = gain_factor;
        sg.apply_gain();
        if (std::isnan(buffer_frequency)) {
            sg.source->set_pitch(pitch);
        } else {
            update_pitch_unsafe(pitch);
        }
    } else {
        auto &sg = sources_.emplace_back(AudioSourceAndGain{
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
            update_pitch_unsafe(pitch);
        }
        sg.source->play();
    }
}

void CrossFade::stop() {
    std::scoped_lock lock{ mutex_ };
    sources_.clear();
}

void CrossFade::set_position(const AudioSourceState<ScenePos>& position) {
    std::scoped_lock lock{ mutex_ };
    for (auto& s : sources_) {
        s.source->set_position(position);
    }
}

void CrossFade::update_gain_unsafe(float dgain) {
    if (sources_.empty()) {
        return;
    }
    auto &sg_back = sources_.back();
    float dgain1 = std::min(sg_back.gain + dgain, 1.f) - sg_back.gain;
    sg_back.gain += dgain1;
    total_gain_ += dgain1;
    sources_.remove_if([&](AudioSourceAndGain &sg) {
        auto excess_gain = total_gain_ - 1.f;
        if (excess_gain <= 0.f) {
            return false;
        }
        auto sg_gain_old = sg.gain;
        sg.gain -= std::min({ sg.gain, dgain1, excess_gain });
        total_gain_ -= sg_gain_old - sg.gain;
        if (sg.gain < EPS) {
            return true;
        }
        sg.apply_gain();
        return false;
    });
    if (total_gain_ > 1.f + EPS) {
        verbose_abort("Cross-fade internal error");
    }
}

void CrossFade::update_pitch_unsafe(float pitch) {
    for (auto& s : sources_) {
        if (!std::isnan(s.buffer_frequency)) {
            s.source->set_pitch(pitch / s.buffer_frequency);
        }
    }
}

void CrossFade::print(std::ostream& ostr) const {
    std::scoped_lock lock{ mutex_ };
    print_unsafe(ostr);
}

void CrossFade::print_unsafe(std::ostream& ostr) const {
    for (const auto& s : sources_) {
        ostr << " gain: " << s.gain;
    }
}
