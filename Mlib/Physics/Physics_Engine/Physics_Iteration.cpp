#include "Physics_Iteration.hpp"
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine/Beacons.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

PhysicsIteration::PhysicsIteration(
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    Scene& scene,
    DynamicWorld& dynamic_world,
    PhysicsEngine& physics_engine,
    DeleteNodeMutex& delete_node_mutex,
    const PhysicsEngineConfig& physics_cfg,
    BaseLog* base_log)
    : scene_node_resources_{ scene_node_resources }
    , rendering_resources_{ rendering_resources }
    , scene_{ scene }
    , dynamic_world_{ dynamic_world }
    , physics_engine_{ physics_engine }
    , delete_node_mutex_{ delete_node_mutex }
    , physics_cfg_{ physics_cfg }
    , base_log_{ base_log }
{}

PhysicsIteration::~PhysicsIteration() = default;

void PhysicsIteration::operator()(std::chrono::steady_clock::time_point time) {
    StaticWorld world{
        .geographic_mapping = dynamic_world_.get_geographic_mapping(),
        .inverse_geographic_mapping = dynamic_world_.get_inverse_geographic_mapping(),
        .gravity = dynamic_world_.get_gravity(),
        .wind = dynamic_world_.get_wind()
    };
    // Note that g_beacons is delayed by one frame.
    std::list<Beacon> beacons = std::move(get_beacons());
    for (const auto& g : physics_engine_.rigid_bodies_.collision_groups()) {
        auto idt = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>(physics_cfg_.dt_substeps_(g.nsubsteps) / seconds));
        for (size_t i = 0; i < g.nsubsteps; ++i) {
            std::list<Beacon>* bcns = (i == g.nsubsteps - 1)
                ? &beacons
                : nullptr;
            world.time = time - (g.nsubsteps - 1 - i) * idt;
            auto phase = PhysicsPhase{
                .burn_in = false,
                .substep = i,
                .group = g
            };
            physics_engine_.compute_transformed_objects(&phase);
            physics_engine_.collide(
                world,
                bcns,
                phase,
                base_log_);
            physics_engine_.move_rigid_bodies(
                world,
                bcns,
                phase);
        }
    }
    physics_engine_.move_particles(world);
    physics_engine_.compute_transformed_objects(nullptr);
    {
        scene_.notify_cleanup_required();
        DestructionGuard dg{ [&]() { scene_.notify_cleanup_done(); } };
        // for(size_t i = 0; i < 32; ++i) {
        //     beacons.push_back(Beacon{.position = p_q2o(g_dest_origin[i]), .resource_name = "flag"});
        // }
        {
            for (const auto& name : beacon_nodes_) {
                scene_.delete_root_node(name);
            }
            beacon_nodes_.clear();
            size_t i = 0;
            for (const auto& beacon : beacons) {
                auto node = make_unique_scene_node(
                    beacon.location.t,
                    matrix_2_tait_bryan_angles<float>(beacon.location.R),
                    beacon.location.get_scale(),
                    PoseInterpolationMode::DISABLED);
                scene_node_resources_.instantiate_child_renderable(
                    beacon.resource_name,
                    ChildInstantiationOptions{
                        .rendering_resources = &rendering_resources_,
                        .instance_name = VariableAndHash<std::string>{ "beacon" },
                        .scene_node = node.ref(DP_LOC),
                        .interpolation_mode = PoseInterpolationMode::DISABLED,
                        .renderable_resource_filter = RenderableResourceFilter{}});
                // node->set_scale(0.05);
                auto& node_name = beacon_nodes_.emplace_back("beacon" + std::to_string(i));
                scene_.auto_add_root_node(node_name, std::move(node), RenderingDynamics::MOVING);
                ++i;
            }
        }
        // TimeGuard tg1{"scene.move"};
        scene_.delete_scheduled_root_nodes();
        scene_.move(physics_cfg_.dt, time);
    }
    physics_engine_.move_advance_times(world);
}
