#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <atomic>
#include <mutex>
#include <string>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Scene;

class DeletingDamageable: public Damageable, public AdvanceTime, public StatusWriter {
    DeletingDamageable(const DeletingDamageable&) = delete;
    DeletingDamageable& operator = (const DeletingDamageable&) = delete;
public:
    explicit DeletingDamageable(
        Scene& scene,
        AdvanceTimes& advance_times,
        std::string root_node_name,
        float health,
        bool delete_node_when_health_leq_zero);
    virtual ~DeletingDamageable() override;
    // AdvanceTime
    virtual void advance_time(float dt) override;
    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;
    virtual float get_value(StatusComponents log_components) const override;
    virtual StatusWriter& child_status_writer(const std::vector<std::string>& name) override;
    // Damageable
    virtual float health() const override;
    virtual void damage(float amount) override;
protected:
    Scene& scene_;
    AdvanceTimes& advance_times_;
    std::string root_node_name_;
    float health_;
    mutable SafeSharedMutex health_mutex_;
    bool delete_node_when_health_leq_zero_;
    DestructionFunctionsRemovalTokens node_on_clear_;
    DestructionGuards dgs_;
};

}
