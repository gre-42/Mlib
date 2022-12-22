#include "Rigid_Body_Playback.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

RigidBodyPlayback::RigidBodyPlayback(
    const std::string& filename,
    AdvanceTimes& advance_times,
    const Focuses& focuses,
    const TransformationMatrix<double, double, 3>* geographic_mapping,
    float speedup)
: advance_times_{advance_times},
  focuses_{focuses},
  speedup_{speedup},
  track_reader_{filename, 1, geographic_mapping}  // 1 = nlaps
{}

RigidBodyPlayback::~RigidBodyPlayback()
{}

void RigidBodyPlayback::advance_time(float dt) {
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    TrackElement track_element;
    size_t nlaps;
    if (track_reader_.read(track_element, nlaps, dt * speedup_)) {
        transformation_matrix_.R() = tait_bryan_angles_2_matrix(track_element.rotation);
        transformation_matrix_.t() = track_element.position;
    }
}

void RigidBodyPlayback::notify_destroyed(Object* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void RigidBodyPlayback::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, double, 3> RigidBodyPlayback::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}
