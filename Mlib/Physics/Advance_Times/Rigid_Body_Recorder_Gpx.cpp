#include "Rigid_Body_Recorder_Gpx.hpp"
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodyRecorderGpx::RigidBodyRecorderGpx(
    const std::string& filename,
    DanglingRef<SceneNode> recorded_node,
    RigidBodyPulses& rbp,
    const TransformationMatrix<double, double, 3>* geographic_coordinates,
    const Focuses& focuses)
    : focuses_{ focuses }
    , recorded_node_{ recorded_node.ptr() }
    , rbp_{ &rbp }
    , geographic_coordinates_{ geographic_coordinates }
    , track_writer_{ filename }
    , start_time_{ std::chrono::steady_clock::now() }
{
    recorded_node_->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

RigidBodyRecorderGpx::~RigidBodyRecorderGpx() {
    on_destroy.clear();
}

void RigidBodyRecorderGpx::advance_time(float dt, const StaticWorld& world) {
    if (recorded_node_ == nullptr) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    if (geographic_coordinates_ == nullptr) {
        THROW_OR_ABORT("RigidBodyRecorderGpx::advance_time without geographic mapping");
    }
    track_writer_.write(geographic_coordinates_->transform(rbp_->abs_position().casted<double>()));
}

void RigidBodyRecorderGpx::notify_destroyed(SceneNode& destroyed_object) {
    rbp_ = nullptr;
    recorded_node_ = nullptr;
    global_object_pool.remove(this);
}
