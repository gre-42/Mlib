#include "Gun.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

Gun::Gun(
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    RigidBodies& rigid_bodies,
    AdvanceTimes& advance_times,
    float cool_down,
    const RigidBodyIntegrator& parent_rbi,
    SceneNode& punch_angle_node,
    const std::string& bullet_renderable_resource_name,
    const std::string& bullet_hitbox_resource_name,
    float bullet_mass,
    float bullet_velocity,
    float bullet_lifetime,
    float bullet_damage,
    const FixedArray<float, 3>& bullet_size,
    float punch_angle,
    DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  scene_node_resources_{ scene_node_resources },
  rigid_bodies_{ rigid_bodies },
  advance_times_{ advance_times },
  parent_rbi_{ parent_rbi },
  punch_angle_node_{ punch_angle_node },
  bullet_renderable_resource_name_{ bullet_renderable_resource_name },
  bullet_hitbox_resource_name_{ bullet_hitbox_resource_name },
  bullet_mass_{ bullet_mass },
  bullet_velocity_{ bullet_velocity },
  bullet_lifetime_{ bullet_lifetime },
  bullet_damage_{ bullet_damage },
  bullet_size_{ bullet_size },
  triggered_{ false },
  cool_down_{ cool_down },
  seconds_since_last_shot_{ 0 },
  absolute_model_matrix_{fixed_nans<float, 4, 4 >() },
  delete_node_mutex_{ delete_node_mutex },
  punch_angle_{ 0.f, 0.f, 0.f },
  rng_{ 0, 0.f, punch_angle }
{}

void Gun::advance_time(float dt) {
    seconds_since_last_shot_ += dt;
    seconds_since_last_shot_ = std::min(seconds_since_last_shot_, cool_down_);
    if ((seconds_since_last_shot_ == cool_down_) && triggered_) {
        seconds_since_last_shot_ = 0;
        triggered_ = false;
        generate_bullet();
    }
    punch_angle_node_.set_rotation(punch_angle_);
    punch_angle_ *= 0.95f;
}

void Gun::generate_bullet() {
    std::shared_ptr<RigidBodyVehicle> rc = rigid_cuboid(bullet_mass_, bullet_size_);
    auto node = std::make_unique<SceneNode>();
    FixedArray<float, 3> t = absolute_model_matrix_.t();
    FixedArray<float, 3> r = matrix_2_tait_bryan_angles(absolute_model_matrix_.R());
    node->set_position(t);
    node->set_rotation(r);
    node->set_absolute_movable(rc.get());
    rc->rbi_.rbp_.v_ =
        - bullet_velocity_ * z3_from_3x3(absolute_model_matrix_.R())
        + parent_rbi_.rbp_.v_;
    scene_node_resources_.instantiate_renderable(
        bullet_renderable_resource_name_,
        "bullet",
        *node,
        RenderableResourceFilter());
    rigid_bodies_.add_rigid_body(
        rc,
        scene_node_resources_.get_animated_arrays(bullet_hitbox_resource_name_)->cvas,
        CollidableMode::SMALL_MOVING,
        PhysicsResourceFilter());
    std::string bullet_node_name = "bullet-" + std::to_string(scene_.get_uuid());
    auto bullet = std::make_shared<Bullet>(
        scene_,
        scene_node_resources_,
        *node,
        advance_times_,
        *rc,
        bullet_node_name,
        bullet_lifetime_,
        bullet_damage_,
        delete_node_mutex_);
    rc->collision_observers_.push_back(bullet);
    advance_times_.add_advance_time(bullet);
    scene_.add_root_node(bullet_node_name, std::move(node));
    punch_angle_ += FixedArray<float, 3>{ rng_(), rng_(), 0.f };

}

void Gun::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix)
{
    absolute_model_matrix_ = absolute_model_matrix;
}

void Gun::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void Gun::trigger() {
    if (is_none_gun()) {
        return;
    }
    if (seconds_since_last_shot_ == cool_down_) {
        triggered_ = true;
    }
}

const TransformationMatrix<float, 3>& Gun::absolute_model_matrix() const {
    return absolute_model_matrix_;
}

bool Gun::is_none_gun() const {
    return bullet_lifetime_ == 0;
}

const FixedArray<float, 3>& Gun::punch_angle() const {
    return punch_angle_;
}
