#include "Load_Scene_Instance_Function.hpp"
#include <Mlib/Scene/Renderable_Scene.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::LoadSceneInstanceFunction(RenderableScene& renderable_scene)
: scene_node_resources{ renderable_scene.scene_node_resources_ },
  players{ renderable_scene.players_ },
  scene{ renderable_scene.scene_ },
  physics_engine{ renderable_scene.physics_engine_ },
  supply_depots{ renderable_scene.supply_depots_ },
  button_press{ renderable_scene.button_press_ },
  cursor_states{ renderable_scene.cursor_states_ },
  scroll_wheel_states{ renderable_scene.scroll_wheel_states_ },
  key_bindings{ *renderable_scene.key_bindings_ },
  selected_cameras{ renderable_scene.selected_cameras_ },
  scene_config{ renderable_scene.scene_config_ },
  render_logics{ renderable_scene.render_logics_ },
  audio_paused{ renderable_scene.audio_paused_ },
  physics_set_fps{ renderable_scene.physics_set_fps_ },
  scene_logic{ renderable_scene.standard_camera_logic_ },
  read_pixels_logic{ renderable_scene.read_pixels_logic_ },
  dirtmap_logic{ *renderable_scene.dirtmap_logic_ },
  standard_render_logic{ *renderable_scene.standard_render_logic_ },
  post_processing_logic{ *renderable_scene.post_processing_logic_ },
  skybox_logic{ renderable_scene.skybox_logic_ },
  game_logic{ renderable_scene.game_logic_ },
  base_log{ renderable_scene.fifo_log_ },
  delete_node_mutex{ renderable_scene.delete_node_mutex_ },
  primary_rendering_context{ renderable_scene.primary_rendering_context_ },
  secondary_rendering_context{ renderable_scene.secondary_rendering_context_ },
  rrg0_{ renderable_scene.primary_rendering_context_ },
  rrg1_{ renderable_scene.secondary_rendering_context_ },
  arg0_{ renderable_scene.primary_audio_resource_context_ },
  arg1_{ renderable_scene.secondary_audio_resource_context_ }
{}

LoadSceneInstanceFunction::~LoadSceneInstanceFunction()
{}
