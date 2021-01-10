#include "Keep_Offset_Movable.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

KeepOffsetMovable::KeepOffsetMovable(
    AdvanceTimes& advance_times,
    Scene& scene,
    const std::string& follower_name,
    SceneNode* followed_node,
    AbsoluteMovable* followed,
    const FixedArray<float, 3>& offset)
: advance_times_{advance_times},
  scene_{scene},
  follower_name_{follower_name},
  followed_node_{followed_node},
  followed_{followed},
  offset_{offset}
{
    followed_node_->add_destruction_observer(this);
}

void KeepOffsetMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    position_ = followed_->get_new_absolute_model_matrix().t() + offset_;
}

void KeepOffsetMovable::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) {
    position_ = t3_from_4x4(absolute_model_matrix);
    rotation_ = R3_from_4x4(absolute_model_matrix);
}

TransformationMatrix<float> KeepOffsetMovable::get_new_absolute_model_matrix() const {
    return TransformationMatrix<float>{rotation_, position_};
}

void KeepOffsetMovable::notify_destroyed(void* obj) {
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
