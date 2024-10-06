#include "Keep_Offset_From_Movable.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

KeepOffsetFromMovable::KeepOffsetFromMovable(
    AdvanceTimes& advance_times,
    Scene& scene,
    const std::string& follower_name,
    DanglingRef<SceneNode> followed_node,
    IAbsoluteMovable& followed,
    const FixedArray<float, 3>& offset)
    : advance_times_{ advance_times }
    , scene_{ scene }
    , follower_name_{ follower_name }
    , followed_node_{ followed_node.ptr() }
    , followed_{ &followed }
    , offset_{ offset }
    , transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
{
    followed_node_->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

KeepOffsetFromMovable::~KeepOffsetFromMovable()
{}

void KeepOffsetFromMovable::advance_time(float dt, const StaticWorld& world) {
    if (followed_ == nullptr) {
        return;
    }
    transformation_matrix_.t = followed_->get_new_absolute_model_matrix().t + offset_.casted<ScenePos>();
}

void KeepOffsetFromMovable::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, ScenePos, 3> KeepOffsetFromMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void KeepOffsetFromMovable::notify_destroyed(SceneNode& destroyed_object) {
    if (&destroyed_object == followed_node_.get()) {
        followed_node_ = nullptr;
        followed_ = nullptr;
        if (!follower_name_.empty()) {
            std::string fn = follower_name_;
            follower_name_.clear();
            scene_.delete_root_node(fn);
        }
    } else {
        if (followed_node_ != nullptr) {
            followed_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        }
        follower_name_.clear();
        global_object_pool.remove(this);
    }
}
