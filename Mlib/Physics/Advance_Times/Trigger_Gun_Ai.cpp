#include "Trigger_Gun_Ai.hpp"
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

TriggerGunAi::TriggerGunAi(
    SceneNode& base_shooter_node,
    SceneNode& base_target_node,
    SceneNode& gun_node,
    RigidBodyIntegrator& rbi_shooter,
    RigidBodyIntegrator& rbi_target,
    PhysicsEngine& physics_engine,
    Gun& gun)
: base_shooter_node_{base_shooter_node},
  base_target_node_{base_target_node},
  gun_node_{gun_node},
  rbi_shooter_{rbi_shooter},
  rbi_target_{rbi_target},
  physics_engine_{physics_engine},
  gun_{gun}
{
    base_shooter_node_.add_destruction_observer(this);
    base_target_node.add_destruction_observer(this);
    gun_node.add_destruction_observer(this);
}

void TriggerGunAi::notify_destroyed(void* destroyed_object) {
    if (destroyed_object == &base_shooter_node_) {
        base_target_node_.remove_destruction_observer(this);
        gun_node_.remove_destruction_observer(this);
    }
    if (destroyed_object == &base_target_node_) {
        base_shooter_node_.remove_destruction_observer(this);
        gun_node_.remove_destruction_observer(this);
    }
    if (destroyed_object == &gun_node_) {
        base_target_node_.remove_destruction_observer(this);
        base_shooter_node_.remove_destruction_observer(this);
    }
    physics_engine_.advance_times_.schedule_delete_advance_time(this);
}

void TriggerGunAi::advance_time(float dt) {
    if (physics_engine_.collision_query_.can_see(rbi_shooter_, rbi_target_)) {
        gun_.trigger();
    }
}
