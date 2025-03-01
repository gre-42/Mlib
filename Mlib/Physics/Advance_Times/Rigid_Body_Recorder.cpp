#include "Rigid_Body_Recorder.hpp"
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

RigidBodyRecorder::RigidBodyRecorder(
    const std::string& filename,
    const TransformationMatrix<double, double, 3>* geographic_mapping,
    DanglingRef<SceneNode> recorded_node,
    RigidBodyPulses& rbp,
    const Focuses& focuses)
    : focuses_{ focuses }
    , recorded_node_{ recorded_node.ptr() }
    , rbp_{ &rbp }
    , track_writer_{ filename, geographic_mapping }
    , start_time_{ std::chrono::steady_clock::now() }
{
    recorded_node_->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

RigidBodyRecorder::~RigidBodyRecorder() {
    on_destroy.clear();
}

void RigidBodyRecorder::advance_time(float dt, const StaticWorld& world) {
    if (recorded_node_ == nullptr) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    track_writer_.write(TrackElement{
        .elapsed_seconds = std::chrono::duration<float>{std::chrono::steady_clock::now() - start_time_}.count(),
        .transformations = {OffsetAndTaitBryanAngles<float, ScenePos, 3>{rbp_->rotation_, rbp_->abs_position()}}});
}

void RigidBodyRecorder::notify_destroyed(SceneNode& destroyed_object) {
    rbp_ = nullptr;
    recorded_node_ = nullptr;

    global_object_pool.remove(this);
}
