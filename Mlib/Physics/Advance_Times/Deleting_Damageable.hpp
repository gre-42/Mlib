#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace Mlib {

class AdvanceTimes;
class Scene;
class DeleteNodeMutex;

class DeletingDamageable: public Damageable, public DestructionObserver, public AdvanceTime, public StatusWriter {
public:
    explicit DeletingDamageable(
        Scene& scene,
        AdvanceTimes& advance_times,
        const std::string& root_node_name,
        float health,
        bool delete_node_when_health_leq_zero,
        DeleteNodeMutex& delete_node_mutex);
    virtual void notify_destroyed(Object& obj) override;
    virtual void advance_time(float dt) override;
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;
    virtual float health() const override;
    virtual void damage(float amount) override;
protected:
    Scene& scene_;
    AdvanceTimes& advance_times_;
    std::string root_node_name_;
    float health_;
    mutable std::shared_mutex health_mutex_;
    bool delete_node_when_health_leq_zero_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
