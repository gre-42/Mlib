#include "Keep_Offset_From_Camera.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

KeepOffsetFromCamera::KeepOffsetFromCamera(
    AdvanceTimes& advance_times,
    Scene& scene,
    SelectedCameras& cameras,
    const FixedArray<float, 3>& offset,
    const FixedArray<float, 3>& grid,
    DanglingRef<SceneNode> follower_node)
    : advance_times_{ advance_times }
    , scene_{ scene }
    , cameras_{ cameras }
    , offset_{ offset }
    , grid_{ grid }
    , follower_node_{ follower_node.ptr() }
    , camera_changed_deletion_token_{
        cameras.camera_changed.insert([this]() {
            if (follower_node_ == nullptr) {
                return;
            }
            advance_time(NAN);
            auto new_trafo = get_new_absolute_model_matrix();
            auto old_trafo = follower_node_->absolute_model_matrix();
            follower_node_->set_absolute_pose(
                new_trafo.t(),
                matrix_2_tait_bryan_angles(old_trafo.R()),
                1.f,
                INITIAL_POSE);
            }
        )
    }
{}

KeepOffsetFromCamera::~KeepOffsetFromCamera()
{}

void KeepOffsetFromCamera::advance_time(float dt) {
    auto new_position_abs = scene_.get_node(cameras_.camera_node_name(), DP_LOC)->absolute_model_matrix().t() + offset_.casted<double>();
    if (all(grid_ == 0.f)) {
        transformation_matrix_.t() = new_position_abs;
    } else {
        auto R = transformation_matrix_.R().casted<double>();
        auto diff_rel = dot(new_position_abs, R);
        for (size_t i = 0; i < 3; ++i) {
            if (grid_(i) == 0.f) {
                diff_rel(i) = 0.;
            } else {
                diff_rel(i) = std::remainder(diff_rel(i), (double)grid_(i));
            }
        }
        transformation_matrix_.t() = new_position_abs - dot1d(R, diff_rel);
    }
}

void KeepOffsetFromCamera::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, double, 3> KeepOffsetFromCamera::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void KeepOffsetFromCamera::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    follower_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}
