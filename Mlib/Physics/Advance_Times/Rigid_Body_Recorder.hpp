#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Writer.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class Focuses;
class AdvanceTimes;
class SceneNode;
struct RigidBodyIntegrator;

class RigidBodyRecorder: public DestructionObserver<DanglingRef<const SceneNode>>, public AdvanceTime {
public:
    RigidBodyRecorder(
        const std::string& filename,
        const TransformationMatrix<double, double, 3>* geographic_mapping,
        AdvanceTimes& advance_times,
        DanglingRef<SceneNode> recorded_node,
        RigidBodyIntegrator* rbi,
        const Focuses& focuses);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;

private:
    const Focuses& focuses_;
    AdvanceTimes& advance_times_;
    DanglingPtr<SceneNode> recorded_node_;
    RigidBodyIntegrator* rbi_;
    TrackWriter track_writer_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
