#include "Physics_Loop.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Set_Fps.hpp>
#include <vector>

using namespace Mlib;

PhysicsLoop::PhysicsLoop(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& physics_engine,
    std::recursive_mutex& mutex,
    const PhysicsEngineConfig& physics_cfg,
    SetFps& set_fps,
    size_t nframes,
    BaseLog* base_log)
: exit_physics_{false},
  set_fps_{set_fps},
  physics_thread_{[&, nframes, base_log](){
    size_t nframes2 = nframes;
    while(!exit_physics_) {
        if (nframes2 != SIZE_MAX) {
            if (nframes2-- == 0) {
                break;
            }
        }
        std::list<Beacon> beacons;
        for (size_t i = 0; i < physics_cfg.oversampling; ++i) {
            std::list<Beacon>* bcns = (i == physics_cfg.oversampling - 1)
                ? &beacons
                : nullptr;
            std::list<std::unique_ptr<ContactInfo>> contact_infos;
            physics_engine.collide(bcns, contact_infos, false, base_log);  // false=burn_in
            if (physics_cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                solve_contacts(contact_infos, physics_cfg.dt / physics_cfg.oversampling);
            }
            physics_engine.move_rigid_bodies(bcns);
        }
        {
            std::lock_guard lock{mutex};
            {
                static const std::regex re{"^beacon.*"};
                scene.delete_root_nodes(re);
                size_t i = 0;
                for (const auto& beacon : beacons) {
                    SceneNode* node = new SceneNode;
                    scene.add_root_node("beacon" + std::to_string(i), new SceneNode);
                    scene_node_resources.instantiate_renderable(beacon.resource_name, "box", *node, SceneNodeResourceFilter{});
                    node->set_position(beacon.position);
                    // node->set_scale(0.05);
                    ++i;
                }
            }
            scene.move();
        }
        physics_engine.move_advance_times();
        physics_engine.advance_times_.delete_scheduled_advance_times();
        // std::cerr << rb0->get_new_absolute_model_matrix() << std::endl;
        set_fps.tick(physics_cfg.dt, physics_cfg.max_residual_time, physics_cfg.print_residual_time);
    }
    }}
{}

void PhysicsLoop::stop_and_join() {
    exit_physics_ = true;
    set_fps_.resume();
    physics_thread_.join();
}

void PhysicsLoop::join() {
    physics_thread_.join();
}
