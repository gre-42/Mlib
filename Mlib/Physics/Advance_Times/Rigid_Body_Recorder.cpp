#include "Rigid_Body_Recorder.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RigidBodyRecorder::RigidBodyRecorder(
    const std::string& filename,
    AdvanceTimes& advance_times,
    SceneNode* recorded_node,
    RigidBodyIntegrator* rbi,
    const Focuses& focuses)
: focuses_{focuses},
  advance_times_{advance_times},
  recorded_node_{recorded_node},
  rbi_{rbi},
  track_writer_{filename},
  start_time_{std::chrono::steady_clock::now()}
{
    recorded_node_->add_destruction_observer(this);
}

void RigidBodyRecorder::advance_time(float dt) {
    if (recorded_node_ == nullptr) {
        return;
    }
    if (focuses_.countdown_active()) {
        return;
    }
    auto rotation = matrix_2_tait_bryan_angles(rbi_->rbp_.rotation_);
    track_writer_.write(TrackElement{
        .elapsed_time = std::chrono::duration<float>{std::chrono::steady_clock::now() - start_time_}.count(),
        .position = rbi_->abs_position(),
        .rotation = rotation});
}

void RigidBodyRecorder::notify_destroyed(void* obj) {
    rbi_ = nullptr;
    recorded_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(this);
}
