#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Scene_Graph/Loggable.hpp>
#include <string>

namespace Mlib {

class AdvanceTimes;
class Scene;

class Damageable: public CollisionObserver, public DestructionObserver, public AdvanceTime, public Loggable {
public:
    explicit Damageable(
        Scene& scene,
        AdvanceTimes& advance_times,
        const std::string& root_node_name,
        float health);
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    virtual void log(std::ostream& ostr, unsigned int log_components) const override;
    void damage(float amount);
protected:
    Scene& scene_;
    AdvanceTimes& advance_times_;
    std::string root_node_name_;
    float health_;
};

}
