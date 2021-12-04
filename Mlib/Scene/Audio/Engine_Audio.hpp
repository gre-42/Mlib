#pragma once
#include <Mlib/Audio/Cross_Fade.hpp>
#include <Mlib/Physics/Misc/Engine_Event_Listener.hpp>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;

class EngineAudio: public EngineEventListener {
public:
    explicit EngineAudio(const std::string& resource_name);
    virtual void notify_off() override;
    virtual void notify_idle(float w) override;
    virtual void notify_driving(float w) override;
    virtual void set_position(const FixedArray<float, 3>& position) override;
private:
    std::shared_ptr<AudioBuffer> driving_buffer;
    std::shared_ptr<AudioBuffer> idle_buffer;
    float driving_gain;
    float idle_gain;
    CrossFade cross_fade_;
};

}
