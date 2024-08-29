#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class AdvanceTimes;

class FollowerAi: public IAdvanceTime, public virtual DanglingBaseClass {
public:
    explicit FollowerAi(
        AdvanceTimes& advance_times,
        const DanglingBaseClassRef<RigidBodyVehicle>& follower,
        const DanglingBaseClassRef<RigidBodyVehicle>& followed);
    virtual ~FollowerAi();
    virtual void advance_time(float dt, const StaticWorld& world);
private:
    AdvanceTimes& advance_times_;
    DanglingBaseClassRef<RigidBodyVehicle> follower_;
    DanglingBaseClassPtr<RigidBodyVehicle> followed_;
    DestructionFunctionsRemovalTokens follower_on_destroy_;
    DestructionFunctionsRemovalTokens followed_on_destroy_;
};

}
