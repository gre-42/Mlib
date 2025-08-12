#include "Cross_Fade.hpp"
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <mutex>

using namespace Mlib;

void AudioSourceAndGain::apply_gain() {
    source->set_gain(gain_factor * static_cast<float>(gain));
}

CrossFade::CrossFade(
    PositionRequirement position_requirement,
    std::function<bool()> paused,
    EventEmitter& paused_changed,
    float dgain)
    : position_requirement_{ position_requirement }
    , total_gain_{ 0.f }
    , dgain_{ dgain }
    , paused_{ std::move(paused) }
    , erdt_{ paused_changed.insert([this](){ advance_time(0.f); }) }
{}

CrossFade::~CrossFade() {
    erdt_.reset();
}

void CrossFade::start_background_thread(float dt) {
    std::scoped_lock lock{ mutex_ };
    if (fader_.has_value()) {
        THROW_OR_ABORT("CrossFade background thread already started");
    }
    fader_.emplace([this, dt]() {
        try {
            ThreadInitializer ti{"Audio CrossFade", ThreadAffinity::POOL};
            while (!fader_->get_stop_token().stop_requested()) {
                advance_time(dt);
                Mlib::sleep_for(std::chrono::duration<float>(dt));
            }
        } catch (const std::exception& e) {
            verbose_abort("Exception in cross-fade: " + std::string(e.what()));
        }
    });
}

void CrossFade::advance_time(float dt) {
    bool pause = paused_();
    std::scoped_lock lock{ mutex_ };
    if (pause) {
        for (auto &s : sources_) {
            s.source->pause();
        }
    } else {
        for (auto &s : sources_) {
            s.source->unpause();
        }
        if (dt != 0.f) {
            update_gain_unsafe(dgain_);
        }
    }
}

// From: https://stackoverflow.com/questions/14579957/std-container-c-move-to-front
template <class T>
void move_to_end(std::list<T>& list, typename std::list<T>::iterator it) {
    list.splice(list.end(), list, it);
}

void CrossFade::play(
    const AudioBuffer& audio_buffer,
    float gain_factor,
    float pitch,
    float buffer_frequency,
    float alpha)
{
    if (gain_factor < 0.f) {
        THROW_OR_ABORT("Attempt to set negative audio gain factor");
    }
    if (gain_factor > 1.f) {
        THROW_OR_ABORT("Attempt to set audio gain factor greater 1");
    }
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
        auto& sg = sources_.emplace_back(AudioSourceAndGain{
            .audio_buffer = &audio_buffer,
            .gain = (Gain)0.f,
            .gain_factor = gain_factor,
            .buffer_frequency = buffer_frequency,
            .source = std::make_unique<AudioSource>(audio_buffer, position_requirement_, alpha)});
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
        AudioScene::set_source_transformation(*s.source, position);
    }
}

void CrossFade::update_gain_unsafe(float dgain) {
    if (sources_.empty()) {
        return;
    }
    auto& sg_back = sources_.back();
    auto dgain1 = std::min(sg_back.gain + Gain{ dgain }, Gain{ 1.f }) - sg_back.gain;
    sg_back.gain += dgain1;
    total_gain_ += dgain1;
    sources_.remove_if([&](AudioSourceAndGain &sg) {
        auto excess_gain = total_gain_ - Gain{ 1.f };
        if (excess_gain == (Gain)0.f) {
            return false;
        }
        auto sg_gain_old = sg.gain;
        sg.gain -= std::min({ sg.gain, dgain1, excess_gain });
        total_gain_ -= sg_gain_old - sg.gain;
        if (sg.gain == (Gain)0.f) {
            return true;
        }
        sg.apply_gain();
        return false;
    });
    if (total_gain_ > (Gain)1.f) {
        verbose_abort("Cross-fade internal error");
    }
    // static size_t i = 0;
    // i = (i + 1) % 100;
    // if (i == 0) {
    //     auto log = linfo();
    //     log << total_gain_ << " - ";
    //     print_unsafe(log);
    // }
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
