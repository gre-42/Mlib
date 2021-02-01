#include "Physics_Iteration.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

PhysicsIteration::PhysicsIteration(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& physics_engine,
    std::recursive_mutex& mutex,
    const PhysicsEngineConfig& physics_cfg,
    BaseLog* base_log)
: scene_node_resources_{ scene_node_resources },
  scene_{ scene },
  physics_engine_{ physics_engine },
  mutex_{mutex},
  physics_cfg_{ physics_cfg },
  base_log_{base_log}
{}

PhysicsIteration::~PhysicsIteration()
{}

void PhysicsIteration::operator()() {
    std::list<Beacon> beacons;
    for (size_t i = 0; i < physics_cfg_.oversampling; ++i) {
        std::list<Beacon>* bcns = (i == physics_cfg_.oversampling - 1)
            ? &beacons
            : nullptr;
        std::list<std::unique_ptr<ContactInfo>> contact_infos;
        physics_engine_.collide(bcns, contact_infos, false, base_log_);  // false=burn_in
        if (physics_cfg_.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
            solve_contacts(contact_infos, physics_cfg_.dt / physics_cfg_.oversampling);
        }
        physics_engine_.move_rigid_bodies(bcns);
    }
    {
        std::lock_guard lock{ mutex_ };
        {
            static const DECLARE_REGEX(re, "^beacon.*");
            scene_.delete_root_nodes(re);
            size_t i = 0;
            for (const auto& beacon : beacons) {
                SceneNode* node = new SceneNode;
                scene_.add_root_node("beacon" + std::to_string(i), new SceneNode);
                scene_node_resources_.instantiate_renderable(beacon.resource_name, "box", *node, SceneNodeResourceFilter());
                node->set_position(beacon.position);
                // node->set_scale(0.05);
                ++i;
            }
        }
        // TimeGuard tg1{"scene.move"};
        scene_.delete_scheduled_root_nodes();
        scene_.move(physics_cfg_.dt);
    }
    physics_engine_.move_advance_times();
    physics_engine_.advance_times_.delete_scheduled_advance_times();
}
