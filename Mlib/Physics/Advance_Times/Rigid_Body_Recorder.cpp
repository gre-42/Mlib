#include "Rigid_Body_Recorder.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RigidBodyRecorder::RigidBodyRecorder(
    const std::string& filename,
    AdvanceTimes& advance_times,
    SceneNode* recorded_node,
    RigidBodyIntegrator* rbi,
    const std::list<Focus>& focus)
: focus_{focus},
  advance_times_{advance_times},
  recorded_node_{recorded_node},
  rbi_{rbi},
  start_time_{std::chrono::steady_clock::now()}
{
    ofstr_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ofstr_.open(filename);
    recorded_node_->add_destruction_observer(this);
}

void RigidBodyRecorder::advance_time(float dt) {
    if (recorded_node_ == nullptr) {
        return;
    }
    if (focus_.empty() || (std::find(focus_.begin(), focus_.end(), Focus::COUNTDOWN) != focus_.end())) {
        return;
    }
    auto rotation = matrix_2_tait_bryan_angles(rbi_->rbp_.rotation_);
    ofstr_ <<
        std::chrono::duration<float>{std::chrono::steady_clock::now() - start_time_}.count() << " " <<
        rbi_->abs_position() << " " <<
        rotation << std::endl;
}

void RigidBodyRecorder::notify_destroyed(void* obj) {
    rbi_ = nullptr;
    recorded_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(this);
}
