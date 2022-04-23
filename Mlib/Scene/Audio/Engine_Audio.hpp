#pragma once
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Cross_Fade.hpp>
#endif
#include <Mlib/Physics/Actuators/Engine_Event_Listener.hpp>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;

class EngineAudio: public EngineEventListener {
public:
    explicit EngineAudio(
        const std::string& resource_name,
        const std::atomic_bool& audio_paused);
    virtual void notify_off() override;
    virtual void notify_idle(float w) override;
    virtual void notify_driving(float w) override;
    virtual void set_position(const FixedArray<float, 3>& position) override;
private:
#ifndef WITHOUT_ALUT
    std::shared_ptr<AudioBuffer> driving_buffer;
    std::shared_ptr<AudioBuffer> idle_buffer;
    float driving_gain;
    float idle_gain;
    CrossFade cross_fade_;
#endif
};

}
