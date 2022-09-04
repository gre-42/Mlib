#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Writer_Gpx.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class Focuses;
class AdvanceTimes;
class SceneNode;
struct RigidBodyIntegrator;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class RigidBodyRecorderGpx: public DestructionObserver, public AdvanceTime {
public:
    RigidBodyRecorderGpx(
        const std::string& filename,
        AdvanceTimes& advance_times,
        SceneNode& recorded_node,
        RigidBodyIntegrator* rbi,
        const TransformationMatrix<double, double, 3>* geographic_coordinates,
        const Focuses& focuses);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(Object* obj) override;

private:
    const Focuses& focuses_;
    AdvanceTimes& advance_times_;
    SceneNode* recorded_node_;
    RigidBodyIntegrator* rbi_;
    const TransformationMatrix<double, double, 3>* geographic_coordinates_;
    TrackWriterGpx track_writer_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
