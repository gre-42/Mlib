#include "Physics_Loop.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
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
        std::vector<FixedArray<float, 3>> beacons;
        physics_engine.collide(beacons, false);  // false=burn_in
        physics_engine.move_rigid_bodies(beacons);
        {
            scene.delete_root_nodes(std::regex{"^beacon.*"});
            for(size_t i = 0; i < beacons.size(); ++i) {
                scene.add_root_node("beacon" + std::to_string(i), new SceneNode);
                scene_node_resources.instantiate_renderable("beacon", "box", *scene.get_node("beacon" + std::to_string(i)), SceneNodeResourceFilter{});
                scene.get_node("beacon" + std::to_string(i))->set_position(beacons[i]);
                // scene.get_node("beacon" + std::to_string(i))->set_scale(0.05);
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
