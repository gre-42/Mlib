#include "Rigid_Body_Playback.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RigidBodyPlayback::RigidBodyPlayback(
    const std::string& filename,
    AdvanceTimes& advance_times,
    const std::list<Focus>& focus,
    float speed)
: advance_times_{advance_times},
  focus_{focus},
  track_reader_{filename, speed}
{}

RigidBodyPlayback::~RigidBodyPlayback()
{}

void RigidBodyPlayback::advance_time(float dt) {
    if (focus_.empty() || (std::find(focus_.begin(), focus_.end(), Focus::COUNTDOWN) != focus_.end())) {
        return;
    }
    float time;
    FixedArray<float, 3> rotation;
    if (track_reader_.read(time, transformation_matrix_.t(), rotation)) {
        transformation_matrix_.R() = tait_bryan_angles_2_matrix(rotation);
    }
}

void RigidBodyPlayback::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void RigidBodyPlayback::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, 3> RigidBodyPlayback::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}
