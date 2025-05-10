#pragma once
#include <Mlib/Physics/Actuators/IEngine_Event_Listener.hpp>
#include <list>
#include <memory>

namespace Mlib {

class EngineEventListeners: public IEngineEventListener {
public:
    EngineEventListeners();
    virtual ~EngineEventListeners() override;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) override;
    virtual void set_location(
        const RotatingFrame<SceneDir, ScenePos, 3>& frame) override;
    virtual void advance_time(float dt) override;

    void add(std::shared_ptr<IEngineEventListener> l);
private:
    std::list<std::shared_ptr<IEngineEventListener>> listeners_;
};
    
}
