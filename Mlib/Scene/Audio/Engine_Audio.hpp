#pragma once
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Cross_Fade.hpp>
#endif
#include <Mlib/Physics/Actuators/Engine_Event_Listener.hpp>
#include <functional>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;
class AudioBufferSequence;

class EngineAudio: public EngineEventListener {
public:
    explicit EngineAudio(
        const std::string& resource_name,
        const std::function<bool()>& audio_paused);
    ~EngineAudio();
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) override;
    virtual void set_position(const FixedArray<double, 3>& position) override;
private:
#ifndef WITHOUT_ALUT
    std::shared_ptr<AudioBufferSequence> driving_buffer_sequence_;
    float driving_gain_;
    CrossFade cross_fade_;
#endif
};

}
