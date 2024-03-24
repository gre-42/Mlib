#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Writer_Gpx.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class Focuses;
class AdvanceTimes;
class SceneNode;
class RigidBodyPulses;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class RigidBodyRecorderGpx: public DestructionObserver<DanglingRef<SceneNode>>, public AdvanceTime, public DanglingBaseClass {
public:
    RigidBodyRecorderGpx(
        const std::string& filename,
        AdvanceTimes& advance_times,
        DanglingRef<SceneNode> recorded_node,
        RigidBodyPulses* rbp,
        const TransformationMatrix<double, double, 3>* geographic_coordinates,
        const Focuses& focuses);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;

private:
    const Focuses& focuses_;
    AdvanceTimes& advance_times_;
    DanglingPtr<SceneNode> recorded_node_;
    RigidBodyPulses* rbp_;
    const TransformationMatrix<double, double, 3>* geographic_coordinates_;
    TrackWriterGpx track_writer_;
    std::chrono::steady_clock::time_point start_time_;
};

}
