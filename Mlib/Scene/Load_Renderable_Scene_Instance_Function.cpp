#include "Load_Renderable_Scene_Instance_Function.hpp"
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>

using namespace Mlib;

LoadRenderableSceneInstanceFunction::LoadRenderableSceneInstanceFunction(RenderableScene& renderable_scene)
    : renderable_scene{ renderable_scene }
    , object_pool{ renderable_scene.object_pool_ }
    , rendering_resources{ renderable_scene.physics_scene_->rendering_resources_ }
    , scene_node_resources{ renderable_scene.physics_scene_->scene_node_resources_ }
    , particle_resources{ renderable_scene.physics_scene_->particle_resources_ }
    , air_particles{ renderable_scene.physics_scene_->air_particles_ }
    , skidmark_particles{ renderable_scene.physics_scene_->skidmark_particles_ }
    , sea_spray_particles{ renderable_scene.physics_scene_->sea_spray_particles_ }
    , trail_renderer{ *renderable_scene.physics_scene_->trail_renderer_ }
    , dynamic_lights{ *renderable_scene.physics_scene_->dynamic_lights_ }
    , vehicle_spawners{ renderable_scene.physics_scene_->vehicle_spawners_ }
    , players{ renderable_scene.physics_scene_->players_ }
    , scene{ renderable_scene.physics_scene_->scene_ }
    , dynamic_world{ renderable_scene.physics_scene_->dynamic_world_ }
    , physics_engine{ renderable_scene.physics_scene_->physics_engine_ }
    , deferred_instantiator{ renderable_scene.physics_scene_->deferred_instantiator_ }
    , supply_depots{ renderable_scene.physics_scene_->supply_depots_ }
    , key_bindings{ *renderable_scene.key_bindings_ }
    , selected_cameras{ renderable_scene.selected_cameras_ }
    , scene_config{ renderable_scene.scene_config_ }
    , render_logics{ renderable_scene.render_logics_ }
    , scene_render_logics{ renderable_scene.scene_render_logics_ }
    , paused{ renderable_scene.physics_scene_->paused_ }
    , paused_changed{ renderable_scene.physics_scene_->paused_changed_ }
    , physics_set_fps{ renderable_scene.physics_scene_->physics_set_fps_ }
    , scene_logic{ renderable_scene.standard_camera_logic_ }
    , read_pixels_logic{ renderable_scene.read_pixels_logic_ }
    , dirtmap_logic{ *renderable_scene.dirtmap_logic_ }
    , standard_render_logic{ *renderable_scene.standard_render_logic_ }
    , aggregate_render_logic{ *renderable_scene.aggregate_render_logic_ }
    , post_processing_logic{ *renderable_scene.post_processing_logic_ }
    , skybox_logic{ renderable_scene.skybox_logic_ }
    , game_logic{ renderable_scene.physics_scene_->game_logic_.get() }
    , base_log{ renderable_scene.physics_scene_->fifo_log_ }
    , ui_focus{ renderable_scene.ui_focus_ }
    , delete_node_mutex{ renderable_scene.physics_scene_->delete_node_mutex_ }
    , arg0_{ renderable_scene.physics_scene_->primary_audio_resource_context_ }
    , arg1_{ renderable_scene.physics_scene_->secondary_audio_resource_context_ }
{}

LoadRenderableSceneInstanceFunction::~LoadRenderableSceneInstanceFunction() = default;
