#include "Rigid_Body_Playback.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RigidBodyPlayback::RigidBodyPlayback(
    const std::string& filename,
    AdvanceTimes& advance_times,
    const Focus& focus,
    float speed)
: advance_times_{advance_times},
  focus_{focus},
  track_reader_{filename, speed}
{}

void RigidBodyPlayback::advance_time(float dt) {
    if (focus_ == Focus::COUNTDOWN) {
        return;
    }
    float time;
    FixedArray<float, 3> rotation;
    if (track_reader_.read(time, position_, rotation)) {
        rotation_ = tait_bryan_angles_2_matrix(rotation);
    }
}

void RigidBodyPlayback::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void RigidBodyPlayback::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) {
    position_ = t3_from_4x4(absolute_model_matrix);
    rotation_ = R3_from_4x4(absolute_model_matrix);
}

FixedArray<float, 4, 4> RigidBodyPlayback::get_new_absolute_model_matrix() const {
    return assemble_homogeneous_4x4(rotation_, position_);
}
