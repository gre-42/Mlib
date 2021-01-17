#include "Renderable_Scene.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene/Load_Scene.hpp>

using namespace Mlib;

RenderableScene::RenderableScene(
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const SceneConfig& scene_config,
    ButtonStates& button_states,
    UiFocus& ui_focus,
    std::map<std::string, size_t>& selection_ids,
    GLFWwindow* window,
    RenderLogics& render_logics,
    const RenderableSceneConfig& config,
    std::recursive_mutex& mutex)
: scene_node_resources_{scene_node_resources},
  rendering_resources_{rendering_resources},
  small_sorted_aggregate_renderer_guard_{rendering_resources},
  small_instances_renderer_guard_{rendering_resources},
  large_aggregate_array_renderer_{rendering_resources},
  large_instances_renderer_{rendering_resources},
  // SceneNode destructors require that physics engine is destroyed after scene,
  // => Create PhysicsEngine before Scene
  physics_engine_{scene_config.physics_engine_config},
  scene_{
      &large_aggregate_array_renderer_,
      &large_instances_renderer_},
  selected_cameras_{scene_},
  ui_focus_{ui_focus},
  selection_ids_{selection_ids},
  button_states_{button_states},
  user_object_{
      button_states: button_states,
      cameras: selected_cameras_,
      focus: ui_focus.focus,
      physics_set_fps: &physics_set_fps_},
  gefp_{FixedArray<float, 3>{0, -9.8, 0}},
  standard_camera_logic_{
      scene_,
      selected_cameras_},
  skybox_logic_{standard_camera_logic_, rendering_resources},
  standard_render_logic_{std::make_shared<StandardRenderLogic>(
      scene_,
      config.with_skybox
        ? (RenderLogic&)skybox_logic_
        : (RenderLogic&)standard_camera_logic_)},
  window_{window},
  flying_camera_logic_{std::make_shared<FlyingCameraLogic>(
      window,
      button_states,
      scene_,
      user_object_,
      config.fly,
      config.rotate)},
  button_press_{button_states},
  key_bindings_{std::make_shared<KeyBindings>(
      button_press_,
      config.print_gamepad_buttons,
      selected_cameras_,
      ui_focus.focus,
      scene_)},
  read_pixels_logic_{*standard_render_logic_},
  dirtmap_logic_{std::make_shared<DirtmapLogic>(read_pixels_logic_, rendering_resources)},
  motion_interp_logic_{std::make_shared<MotionInterpolationLogic>(read_pixels_logic_, InterpolationType::OPTICAL_FLOW)},
  post_processing_logic_{std::make_shared<PostProcessingLogic>(
      *standard_render_logic_,
      config.depth_fog,
      config.low_pass,
      config.high_pass)},
  mutex_{mutex},
  render_logics_{render_logics},
  players_{physics_engine_.advance_times_},
  game_logic_{
      scene_,
      physics_engine_.advance_times_,
      players_,
      ui_focus.focus,
      mutex_},
  scene_config_{scene_config}
{
    render_logics_.append(nullptr, flying_camera_logic_);
    if (config.with_dirtmap) {
        render_logics_.append(nullptr, dirtmap_logic_);
    }
    render_logics_.append(nullptr, config.vfx
        ? post_processing_logic_
        : (scene_config.render_config.motion_interpolation
            ? std::dynamic_pointer_cast<RenderLogic>(motion_interp_logic_)
            : standard_render_logic_));
    render_logics_.append(nullptr, key_bindings_);
    physics_engine_.add_external_force_provider(&gefp_);
    physics_engine_.add_external_force_provider(key_bindings_.get());
}

void RenderableScene::start_physics_loop() {
    if (physics_loop_ != nullptr) {
        throw std::runtime_error("physics loop already started");
    }
    physics_loop_.reset(
        new PhysicsLoop{
            scene_node_resources_,
            scene_,
            physics_engine_,
            mutex_,
            scene_config_.physics_engine_config,
            physics_set_fps_,
            SIZE_MAX,  // nframes
            &fifo_log_});
}

void RenderableScene::print_physics_engine_search_time() const {
    physics_engine_.rigid_bodies_.print_search_time();
}

void RenderableScene::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::lock_guard lock{mutex_}; // formerly shared_lock

    // TimeGuard tg0{"render"};
    render_logics_.render(
        width,
        height,
        scene_config_.render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

void RenderableScene::stop_and_join() {
    if (physics_loop_ != nullptr) {
        physics_loop_->stop_and_join();
    }
}
