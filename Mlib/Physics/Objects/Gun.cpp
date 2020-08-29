#include "Gun.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Objects/Bullet.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>
#include <Mlib/Physics/Objects/Rigid_Primitives.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

Gun::Gun(
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    RigidBodies& rigid_bodies,
    AdvanceTimes& advance_times,
    float cool_down,
    const RigidBodyIntegrator& parent_rbi,
    const std::string& bullet_renderable_resource_name,
    const std::string& bullet_hitbox_resource_name,
    float bullet_mass,
    float bullet_velocity,
    float bullet_lifetime,
    float bullet_damage,
    const FixedArray<float, 3>& bullet_size)
: scene_{scene},
  scene_node_resources_{scene_node_resources},
  rigid_bodies_{rigid_bodies},
  advance_times_{advance_times},
  parent_rbi_{parent_rbi},
  bullet_renderable_resource_name_{bullet_renderable_resource_name},
  bullet_hitbox_resource_name_{bullet_hitbox_resource_name},
  bullet_mass_{bullet_mass},
  bullet_velocity_{bullet_velocity},
  bullet_lifetime_{bullet_lifetime},
  bullet_damage_{bullet_damage},
  bullet_size_{bullet_size},
  triggered_{false},
  cool_down_{cool_down},
  seconds_since_last_shot_{0},
  absolute_model_matrix_{fixed_nans<float, 4, 4>()}
{}

void Gun::advance_time(float dt) {
    seconds_since_last_shot_ += dt;
    seconds_since_last_shot_ = std::min(seconds_since_last_shot_, cool_down_);
    if ((seconds_since_last_shot_ == cool_down_) && triggered_) {
        seconds_since_last_shot_ = 0;
        triggered_ = false;
        std::shared_ptr<RigidBody> rc = rigid_cuboid(rigid_bodies_, bullet_mass_, bullet_size_);
        SceneNode* node = new SceneNode;
        FixedArray<float, 3> t = t3_from_4x4(absolute_model_matrix_);
        FixedArray<float, 3> r = matrix_2_tait_bryan_angles(R3_from_4x4(absolute_model_matrix_));
        node->set_position(t);
        node->set_rotation(r);
        node->set_absolute_movable(rc.get());
        rc->rbi_.v_ =
            - bullet_velocity_ * z3_from_4x4(absolute_model_matrix_)
            + parent_rbi_.v_;
        scene_node_resources_.instantiate_renderable(bullet_renderable_resource_name_, "bullet", *node, SceneNodeResourceFilter{});
        rigid_bodies_.add_rigid_body(rc, scene_node_resources_.get_triangle_meshes(bullet_hitbox_resource_name_), {});
        std::string bullet_node_name = "bullet-" + std::to_string(scene_.get_uuid());
        auto bullet = std::make_shared<Bullet>(
            scene_,
            *node,
            advance_times_,
            *rc,
            bullet_node_name,
            bullet_lifetime_,
            bullet_damage_);
        rc->collision_observers_.push_back(bullet);
        advance_times_.add_advance_time(bullet);
        scene_.add_root_node(bullet_node_name, node);
    }
}

void Gun::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix)
{
    absolute_model_matrix_ = absolute_model_matrix;
}

void Gun::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void Gun::trigger() {
    if (seconds_since_last_shot_ == cool_down_) {
        triggered_ = true;
    }
}
