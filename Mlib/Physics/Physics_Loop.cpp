#include "Physics_Loop.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Constraints.hpp>
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
    std::shared_mutex& mutex,
    const PhysicsEngineConfig& physics_cfg,
    SetFps& set_fps,
    size_t nframes)
: exit_physics_{false},
  set_fps_{set_fps},
  physics_thread_{[&, nframes](){
    size_t nframes2 = nframes;
    while(!exit_physics_) {
        if (nframes2 != SIZE_MAX) {
            if (nframes2-- == 0) {
                break;
            }
        }
        std::list<Beacon> beacons;
        for(size_t i = 0; i < physics_cfg.oversampling; ++i) {
            beacons.clear();
            std::list<ContactInfo> contact_infos;
            physics_engine.collide(beacons, contact_infos, false);  // false=burn_in
            if (physics_cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                solve_contacts(contact_infos, physics_cfg.dt, physics_cfg.contact_beta, physics_cfg.contact_beta2);
            }
            physics_engine.move_rigid_bodies(beacons);
        }
        {
            scene.delete_root_nodes(std::regex{"^beacon.*"});
            size_t i = 0;
            for(const auto& beacon : beacons) {
                scene.add_root_node("beacon" + std::to_string(i), new SceneNode);
                scene_node_resources.instantiate_renderable(beacon.resource_name, "box", *scene.get_node("beacon" + std::to_string(i)), SceneNodeResourceFilter{});
                scene.get_node("beacon" + std::to_string(i))->set_position(beacon.position);
                // scene.get_node("beacon" + std::to_string(i))->set_scale(0.05);
                ++i;
            }
        }
        {
            std::lock_guard lock{mutex};
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
