#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
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

class RigidBodyRecorderGpx: public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    RigidBodyRecorderGpx(
        const std::string& filename,
        DanglingRef<SceneNode> recorded_node,
        RigidBodyPulses& rbp,
        const TransformationMatrix<double, double, 3>* geographic_coordinates,
        const Focuses& focuses);
    ~RigidBodyRecorderGpx();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;

private:
    const Focuses& focuses_;
    DanglingPtr<SceneNode> recorded_node_;
    RigidBodyPulses* rbp_;
    const TransformationMatrix<double, double, 3>* geographic_coordinates_;
    TrackWriterGpx track_writer_;
    std::chrono::steady_clock::time_point start_time_;
};

}
