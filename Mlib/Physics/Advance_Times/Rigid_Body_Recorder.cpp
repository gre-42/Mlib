#include "Rigid_Body_Recorder.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Integrator.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

RigidBodyRecorder::RigidBodyRecorder(
    const std::string& filename,
    const TransformationMatrix<double, double, 3>* geographic_mapping,
    AdvanceTimes& advance_times,
    SceneNode& recorded_node,
    RigidBodyIntegrator* rbi,
    const Focuses& focuses)
: focuses_{focuses},
  advance_times_{advance_times},
  recorded_node_{&recorded_node},
  rbi_{rbi},
  track_writer_{filename, geographic_mapping},
  start_time_{std::chrono::steady_clock::now()}
{
    recorded_node_->destruction_observers.add(this);
}

void RigidBodyRecorder::advance_time(float dt) {
    if (recorded_node_ == nullptr) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    auto rotation = matrix_2_tait_bryan_angles(rbi_->rbp_.rotation_);
    track_writer_.write(TrackElement{
        .elapsed_seconds = std::chrono::duration<float>{std::chrono::steady_clock::now() - start_time_}.count(),
        .position = rbi_->abs_position(),
        .rotation = rotation});
}

void RigidBodyRecorder::notify_destroyed(Object* obj) {
    rbi_ = nullptr;
    recorded_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(this);
}
