#include "Keep_Offset_From_Camera.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

KeepOffsetFromCamera::KeepOffsetFromCamera(
    AdvanceTimes& advance_times,
    Scene& scene,
    const SelectedCameras& cameras,
    const FixedArray<float, 3>& offset)
: advance_times_{advance_times},
  scene_{scene},
  cameras_{cameras},
  offset_{offset}
{}

KeepOffsetFromCamera::~KeepOffsetFromCamera()
{}

void KeepOffsetFromCamera::advance_time(float dt) {
    transformation_matrix_.t() = scene_.get_node(cameras_.camera_node_name()).absolute_model_matrix().t() + offset_.casted<double>();
}

void KeepOffsetFromCamera::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, double, 3> KeepOffsetFromCamera::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void KeepOffsetFromCamera::notify_destroyed(Object* obj) {
    advance_times_.schedule_delete_advance_time(this);
}
