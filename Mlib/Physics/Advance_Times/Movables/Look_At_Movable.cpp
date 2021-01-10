#include "Look_At_Movable.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

LookAtMovable::LookAtMovable(
    AdvanceTimes& advance_times,
    Scene& scene,
    const std::string& follower_name,
    SceneNode* followed_node,
    AbsoluteMovable* followed)
: advance_times_{advance_times},
  scene_{scene},
  follower_name_{follower_name},
  followed_node_{followed_node},
  followed_{followed}
{
    followed_node_->add_destruction_observer(this);
}

void LookAtMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    auto dmat = followed_->get_new_absolute_model_matrix();
    auto dpos = dmat.t();
    rotation_ = lookat(position_, dpos);
}

void LookAtMovable::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) {
    position_ = t3_from_4x4(absolute_model_matrix);
    rotation_ = R3_from_4x4(absolute_model_matrix);
}

TransformationMatrix<float> LookAtMovable::get_new_absolute_model_matrix() const {
    return TransformationMatrix<float>{rotation_, position_};
}

void LookAtMovable::notify_destroyed(void* obj) {
    if (obj == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
        if (!follower_name_.empty() && !scene_.shutting_down()) {
            std::string fn = follower_name_;
            follower_name_.clear();
            scene_.delete_root_node(fn);
        }
    } else {
        if (followed_node_ != nullptr) {
            followed_node_->remove_destruction_observer(this);
        }
        advance_times_.schedule_delete_advance_time(this);
        follower_name_.clear();
    }
}
