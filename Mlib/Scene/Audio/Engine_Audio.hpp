#pragma once
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Cross_Fade.hpp>
#endif
#include <Mlib/Physics/Actuators/Engine_Event_Listener.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <functional>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;
class AudioBufferSequenceWithHysteresis;

class EngineAudio: public EngineEventListener {
public:
    explicit EngineAudio(
        const std::string& resource_name,
        const std::function<bool()>& audio_paused,
        float p_reference,
        float p_idle);
    ~EngineAudio();
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) override;
    virtual void set_position(const AudioSourceState<ScenePos>& position) override;
private:
#ifndef WITHOUT_ALUT
    std::shared_ptr<AudioBufferSequenceWithHysteresis> driving_buffer_sequence_;
    float driving_gain_;
    CrossFade cross_fade_;
    float p_reference_;
    float p_idle_;
#endif
};

}
