#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Writer.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class CountdownPhysics;
class SceneNode;
class RigidBodyPulses;

class RigidBodyRecorder: public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    RigidBodyRecorder(
        const std::string& filename,
        const TransformationMatrix<double, double, 3>* geographic_mapping,
        DanglingRef<SceneNode> recorded_node,
        RigidBodyPulses& rbp,
        const CountdownPhysics* countdown_start);
    ~RigidBodyRecorder();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;

private:
    const CountdownPhysics* countdown_start_;
    DanglingPtr<SceneNode> recorded_node_;
    RigidBodyPulses* rbp_;
    TrackWriter track_writer_;
    std::chrono::steady_clock::time_point start_time_;
};

}
