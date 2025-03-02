#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <mutex>
#include <string>

namespace Mlib {

class AdvanceTimes;
class RigidBodyVehicle;
class SceneNode;
class Scene;
class Translator;

class DeletingDamageable: public IDamageable, public IAdvanceTime, public StatusWriter, public virtual DanglingBaseClass {
    DeletingDamageable(const DeletingDamageable&) = delete;
    DeletingDamageable& operator = (const DeletingDamageable&) = delete;
public:
    explicit DeletingDamageable(
        Scene& scene,
        AdvanceTimes& advance_times,
        std::string root_node_name,
        float health,
        bool delete_node_when_health_leq_zero,
        std::shared_ptr<Translator> translator);
    virtual ~DeletingDamageable() override;
    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;
    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents log_components, const StaticWorld& world) const override;
    virtual float get_value(StatusComponents log_components) const override;
    virtual StatusWriter& child_status_writer(const std::vector<VariableAndHash<std::string>>& name) override;
    // IDamageable
    virtual float health() const override;
    virtual void damage(float amount) override;
protected:
    Scene& scene_;
    AdvanceTimes& advance_times_;
    std::string root_node_name_;
    float health_;
    mutable SafeAtomicRecursiveSharedMutex health_mutex_;
    bool delete_node_when_health_leq_zero_;
    RigidBodyVehicle* rb_;
    std::shared_ptr<Translator> translator_;
    DestructionGuards dgs_;
    DestructionFunctionsRemovalTokens node_on_clear_;
    DestructionFunctionsRemovalTokens rb_on_destroy_;
};

}
