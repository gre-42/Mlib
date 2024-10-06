#include "Keep_Offset_From_Camera.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

KeepOffsetFromCamera::KeepOffsetFromCamera(
    AdvanceTimes& advance_times,
    Scene& scene,
    SelectedCameras& cameras,
    const FixedArray<float, 3>& offset,
    const FixedArray<float, 3>& grid,
    const DanglingRef<SceneNode>& follower_node)
    : advance_times_{ advance_times }
    , scene_{ scene }
    , cameras_{ cameras }
    , offset_{ offset }
    , grid_{ grid }
    , follower_node_{ follower_node.ptr() }
    , transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
    , camera_changed_deletion_token_{
        cameras.camera_changed.insert([this]() {
            if (follower_node_ == nullptr) {
                return;
            }
            set_absolute_model_matrix(follower_node_->absolute_model_matrix());
            advance_time(NAN);
            auto trafo = get_new_absolute_model_matrix();
            follower_node_->set_absolute_pose(
                trafo.t,
                matrix_2_tait_bryan_angles(trafo.R),
                1.f,
                INITIAL_POSE);
            }
        )
    }
{}

KeepOffsetFromCamera::~KeepOffsetFromCamera() {
    on_destroy.clear();
}

void KeepOffsetFromCamera::advance_time(float dt, const StaticWorld& world) {
    advance_time(dt);
}

void KeepOffsetFromCamera::advance_time(float dt) {
    auto new_position_abs = cameras_.camera(DP_LOC).node->absolute_model_matrix().t + offset_.casted<ScenePos>();
    if (all(grid_ == 0.f)) {
        transformation_matrix_.t = new_position_abs;
    } else {
        auto R = transformation_matrix_.R.casted<ScenePos>();
        auto diff_rel = dot(new_position_abs, R);
        for (size_t i = 0; i < 3; ++i) {
            if (grid_(i) == 0.f) {
                diff_rel(i) = 0.;
            } else {
                diff_rel(i) = std::remainder(diff_rel(i), (ScenePos)grid_(i));
            }
        }
        transformation_matrix_.t = new_position_abs - dot1d(R, diff_rel);
    }
}

void KeepOffsetFromCamera::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, ScenePos, 3> KeepOffsetFromCamera::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void KeepOffsetFromCamera::notify_destroyed(SceneNode& destroyed_object) {
    if (destroyed_object.has_absolute_movable()) {
        if (&destroyed_object.get_absolute_movable() != this) {
            verbose_abort("Unexpected absolute movable");
        }
        destroyed_object.clear_absolute_movable();
    }
    follower_node_ = nullptr;
    global_object_pool.remove(this);
}
