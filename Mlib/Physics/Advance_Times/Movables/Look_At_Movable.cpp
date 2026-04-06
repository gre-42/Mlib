
#include "Look_At_Movable.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Throwing_Lock_Guard.hpp>

using namespace Mlib;

LookAtMovable::LookAtMovable(
    Scene& scene,
    VariableAndHash<std::string> follower_name,
    const DanglingBaseClassRef<SceneNode>& follower_node,
    const DanglingBaseClassRef<SceneNode>& followed_node,
    const DanglingBaseClassRef<IAbsoluteMovable>& followed)
    : follower_setter{ *this }
    , followed_setter{ *this }
    , scene_{ scene }
    , follower_name_{ std::move(follower_name) }
    , follower_node_{ follower_node.ptr() }
    , followed_node_{ followed_node.ptr() }
    , followed_{ followed.ptr() }
    , transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
{}

LookAtMovable::~LookAtMovable() {
    if ((follower_node_ != nullptr) && follower_node_->has_absolute_movable()) {
        if (&follower_node_->get_absolute_movable(CURRENT_SOURCE_LOCATION).get() != this) {
            verbose_abort("Unexpected absolute movable");
        }
        follower_node_->clear_absolute_movable();
    }
    follower_node_ = nullptr;
    followed_node_ = nullptr;
    on_destroy.clear();
}

void LookAtMovable::advance_time(float dt, const StaticWorld& world) {
    if (followed_ == nullptr) {
        return;
    }
    auto dmat = followed_->get_new_absolute_model_matrix();
    const auto& dpos = dmat.t;
    auto R = gl_lookat_absolute(transformation_matrix_.t, dpos);
    if (R.has_value()) {
        transformation_matrix_.R = R->casted<float>();
    }
}

void LookAtMovable::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, ScenePos, 3> LookAtMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void LookAtMovable::notify_destroyed(SceneNode& destroyed_object) {
    ThrowingLockGuard lock{scene_.delete_node_mutex};
    if (&destroyed_object == followed_node_.get()) {
        followed_ = nullptr;
        followed_node_ = nullptr;
        if ((follower_node_ != nullptr) &&
            (follower_node_->shutdown_phase() == ShutdownPhase::NONE))
        {
            scene_.delete_root_node(follower_name_);
        }
    }
    if (&destroyed_object == follower_node_.get()) {
        follower_node_->clear_absolute_movable();
        follower_node_ = nullptr;
        global_object_pool.remove(this);
    }
}

LookAtMovableFollowerNodeSetter::LookAtMovableFollowerNodeSetter(LookAtMovable& look_at_movable)
    : look_at_movable_{ look_at_movable }
    , removal_tokens_{ nullptr, CURRENT_SOURCE_LOCATION }
{}

void LookAtMovableFollowerNodeSetter::set_scene_node(
    Scene& scene,
    const DanglingBaseClassRef<SceneNode>& node,
    VariableAndHash<std::string> node_name,
    SourceLocation loc)
{
    removal_tokens_.set(node->on_clear.early, loc);
    removal_tokens_.add([this, node](){
        look_at_movable_.notify_destroyed(node.get());
    }, loc);
    node->set_absolute_movable({ look_at_movable_, loc });
}

LookAtMovableFollowedNodeSetter::LookAtMovableFollowedNodeSetter(LookAtMovable& look_at_movable)
    : look_at_movable_{ look_at_movable }
    , removal_tokens_{ nullptr, CURRENT_SOURCE_LOCATION }
{}

void LookAtMovableFollowedNodeSetter::set_scene_node(
    Scene& scene,
    const DanglingBaseClassRef<SceneNode>& node,
    VariableAndHash<std::string> node_name,
    SourceLocation loc)
{
    removal_tokens_.set(node->on_clear.early, loc);
    removal_tokens_.add([this, node](){
        look_at_movable_.notify_destroyed(node.get());
    }, loc);
}
