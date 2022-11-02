#include "Physics_Iteration.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

thread_local std::list<Beacon> Mlib::g_beacons;

PhysicsIteration::PhysicsIteration(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& physics_engine,
    DeleteNodeMutex& delete_node_mutex,
    const PhysicsEngineConfig& physics_cfg,
    BaseLog* base_log)
: scene_node_resources_{ scene_node_resources },
  scene_{ scene },
  physics_engine_{ physics_engine },
  delete_node_mutex_{ delete_node_mutex },
  physics_cfg_{ physics_cfg },
  base_log_{ base_log }
{}

PhysicsIteration::~PhysicsIteration()
{}

void PhysicsIteration::operator()() {
    // Note that g_beacons is delayed by one frame.
    std::list<Beacon> beacons = std::move(g_beacons);
    for (size_t i = 0; i < physics_cfg_.oversampling; ++i) {
        std::list<Beacon>* bcns = (i == physics_cfg_.oversampling - 1)
            ? &beacons
            : nullptr;
        physics_engine_.collide(
            bcns,
            false,          // false=burn_in
            i,
            base_log_);
        physics_engine_.move_rigid_bodies(bcns);
    }
    {
        // for(size_t i = 0; i < 32; ++i) {
        //     beacons.push_back(Beacon{.position = p_q2o(g_dest_origin[i]), .resource_name = "flag"});
        // }
        std::lock_guard lock{ delete_node_mutex_ };
        {
            static const DECLARE_REGEX(re, "^beacon.*");
            scene_.delete_root_nodes(re);
            size_t i = 0;
            for (const auto& beacon : beacons) {
                auto node = std::make_unique<SceneNode>();
                scene_node_resources_.instantiate_renderable(
                    beacon.resource_name,
                    InstantiationOptions{
                        .instance_name = "beacon",
                        .scene_node = *node,
                        .renderable_resource_filter = RenderableResourceFilter{}});
                node->set_relative_pose(
                    beacon.location.t(),
                    matrix_2_tait_bryan_angles<float>(beacon.location.R()),
                    beacon.location.get_scale());
                // node->set_scale(0.05);
                scene_.add_root_node("beacon" + std::to_string(i), std::move(node));
                ++i;
            }
        }
        // TimeGuard tg1{"scene.move"};
        scene_.delete_scheduled_root_nodes();
        scene_.move(physics_cfg_.dt / s);
    }
    physics_engine_.move_advance_times();
    physics_engine_.advance_times_.delete_scheduled_advance_times();
}

void PhysicsIteration::delete_scheduled_advance_times() {
    physics_engine_.advance_times_.delete_scheduled_advance_times();
}
