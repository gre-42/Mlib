#include "Look_At_Movable.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LookAtMovable::LookAtMovable(
    AdvanceTimes& advance_times,
    Scene& scene,
    const std::string& follower_name,
    SceneNode& followed_node,
    AbsoluteMovable& followed)
: advance_times_{advance_times},
  scene_{scene},
  follower_name_{follower_name},
  followed_node_{&followed_node},
  followed_{&followed}
{}

LookAtMovable::~LookAtMovable()
{}

void LookAtMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    auto dmat = followed_->get_new_absolute_model_matrix();
    auto dpos = dmat.t();
    transformation_matrix_.R() = gl_lookat_absolute(transformation_matrix_.t(), dpos).casted<float>();
}

void LookAtMovable::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, double, 3> LookAtMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void LookAtMovable::notify_destroyed(Object* obj) {
    if (obj == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
        if (!follower_name_.empty()) {
            std::string fn = follower_name_;
            follower_name_.clear();
            scene_.delete_root_node(fn);
        }
    } else {
        if (followed_node_ != nullptr) {
            followed_node_->destruction_observers.remove(this);
        }
        advance_times_.schedule_delete_advance_time(this);
        follower_name_.clear();
    }
}
