#include "Load_Physics_Scene_Instance_Function.hpp"
#include <Mlib/Scene/Physics_Scene.hpp>

using namespace Mlib;

LoadPhysicsSceneInstanceFunction::LoadPhysicsSceneInstanceFunction(PhysicsScene& physics_scene)
    : physics_scene{ physics_scene }
    , object_pool{ physics_scene.object_pool_ }
    , deferred_instantiator{ physics_scene.deferred_instantiator_ }
    , render_logics{ physics_scene.render_logics_ }
    , rendering_resources{ physics_scene.rendering_resources_ }
    , scene_node_resources{ physics_scene.scene_node_resources_ }
    , particle_resources{ physics_scene.particle_resources_ }
    , air_particles{ physics_scene.air_particles_ }
    , skidmark_particles{ physics_scene.skidmark_particles_ }
    , sea_spray_particles{ physics_scene.sea_spray_particles_ }
    , trail_renderer{ *physics_scene.trail_renderer_ }
    , dynamic_lights{ *physics_scene.dynamic_lights_ }
    , one_shot_audio{ physics_scene.one_shot_audio_ }
    , bullet_generator{ physics_scene.bullet_generator_ }
    , vehicle_spawners{ physics_scene.vehicle_spawners_ }
    , players{ physics_scene.players_ }
    , scene{ physics_scene.scene_ }
    , dynamic_world{ physics_scene.dynamic_world_ }
    , physics_engine{ physics_scene.physics_engine_ }
    , supply_depots{ physics_scene.supply_depots_ }
    , scene_config{ physics_scene.scene_config_ }
    , paused{ physics_scene.paused_ }
    , paused_changed{ physics_scene.paused_changed_ }
    , physics_set_fps{ physics_scene.physics_set_fps_ }
    , game_logic{ physics_scene.game_logic_.get() }
    , countdown_start{ physics_scene.countdown_start_ }
    , ui_focus{ physics_scene.ui_focus_ }
    , base_log{ physics_scene.fifo_log_ }
    , delete_node_mutex{ physics_scene.delete_node_mutex_ }
    , arg0_{ physics_scene.primary_audio_resource_context_ }
    , arg1_{ physics_scene.secondary_audio_resource_context_ }
{}

LoadPhysicsSceneInstanceFunction::~LoadPhysicsSceneInstanceFunction() = default;
