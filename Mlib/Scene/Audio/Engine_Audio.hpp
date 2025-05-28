#pragma once
#include <Mlib/Audio/Cross_Fade.hpp>
#include <Mlib/Physics/Actuators/IEngine_Event_Listener.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <functional>
#include <memory>
#include <string>

namespace Mlib {

class EventEmitter;
class AudioBuffer;
class AudioBufferSequenceWithHysteresis;

class EngineAudio: public IEngineEventListener {
public:
    explicit EngineAudio(
        const std::string& resource_name,
        std::function<bool()> audio_paused,
        EventEmitter& audio_paused_changed,
        float p_reference,
        float p_idle);
    virtual ~EngineAudio() override;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) override;
    virtual void set_location(
        const RotatingFrame<SceneDir, ScenePos, 3>& frame) override;
    virtual void advance_time(float dt) override;
private:
    std::shared_ptr<AudioBufferSequenceWithHysteresis> driving_buffer_sequence_;
    float driving_gain_;
    CrossFade cross_fade_;
    float p_reference_;
    float p_idle_;
};

}
