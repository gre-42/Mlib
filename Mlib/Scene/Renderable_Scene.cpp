#include "Renderable_Scene.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Physics_Loop.hpp>
#include <Mlib/Players/Advance_Times/Pod_Bots.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fxaa_Logic.hpp>
#include <Mlib/Render/Render_Logics/Motion_Interpolation_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/Audio/Audio_Listener_Updater.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>

using namespace Mlib;

RenderableScene::RenderableScene(
    SceneNodeResources& scene_node_resources,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    GLFWwindow* window,
    const SceneConfigResource& config,
    const std::string& level_name,
    size_t max_tracks)
: scene_node_resources_{scene_node_resources},
  // SceneNode destructors require that physics engine is destroyed after scene,
  // => Create PhysicsEngine before Scene
  physics_engine_{scene_config.physics_engine_config},
  scene_{
      delete_node_mutex_,
      &scene_node_resources},
  selected_cameras_{scene_},
  button_states_{button_states},
  cursor_states_{cursor_states},
  scroll_wheel_states_{scroll_wheel_states},
  user_object_{
      .base_user_object{
          .fullscreen_width = scene_config.render_config.fullscreen_width,
          .fullscreen_height = scene_config.render_config.fullscreen_height,
      },
      .button_states = button_states,
      .cursor_states = cursor_states,
      .scroll_wheel_states = scroll_wheel_states,
      .cameras = selected_cameras_,
      .focuses = ui_focus.focuses,
      .wire_frame = scene_config.render_config.wire_frame,
      .depth_test = scene_config.render_config.depth_test,
      .cull_faces = scene_config.render_config.cull_faces,
      .delete_node_mutex = delete_node_mutex_,
      .physics_set_fps = &physics_set_fps_},
  gefp_{gravity_vector},
  standard_camera_logic_{
      scene_,
      selected_cameras_,
      delete_node_mutex_},
  skybox_logic_{standard_camera_logic_},
  standard_render_logic_{std::make_shared<StandardRenderLogic>(
      scene_,
      config.with_skybox
        ? (RenderLogic&)skybox_logic_
        : (RenderLogic&)standard_camera_logic_,
      config.background_color,
      config.clear_mode)},
  window_{window},
  flying_camera_logic_{config.with_flying_logic
      ? std::make_shared<FlyingCameraLogic>(
        window,
        button_states,
        scene_,
        user_object_,
        config.fly,
        config.rotate)
      : nullptr},
  button_press_{button_states},
  key_bindings_{std::make_shared<KeyBindings>(
      button_press_,
      config.print_gamepad_buttons,
      selected_cameras_,
      ui_focus.focuses)},
  read_pixels_logic_{*standard_render_logic_},
  dirtmap_logic_{std::make_shared<DirtmapLogic>(read_pixels_logic_)},
  motion_interp_logic_{std::make_shared<MotionInterpolationLogic>(read_pixels_logic_, InterpolationType::OPTICAL_FLOW)},
  post_processing_logic_{std::make_shared<PostProcessingLogic>(
      *motion_interp_logic_,
      config.background_color,
      config.depth_fog,
      config.low_pass,
      config.high_pass)},
  fxaa_logic_{std::make_shared<FxaaLogic>(*post_processing_logic_)},
  render_logics_{delete_node_mutex_, ui_focus},
  players_{physics_engine_.advance_times_, level_name, max_tracks, scene_node_resources},
  supply_depots_{players_, scene_config.physics_engine_config},
  pod_bots_{config.with_pod_bot
        ? std::make_unique<PodBots>(
            physics_engine_.advance_times_,
            players_,
            physics_engine_.collision_query_,
            delete_node_mutex_)
        : nullptr},
  game_logic_{
      scene_,
      physics_engine_.advance_times_,
      players_,
      supply_depots_,
      delete_node_mutex_},
  scene_config_{scene_config},
  physics_iteration_{
      scene_node_resources_,
      scene_,
      physics_engine_,
      delete_node_mutex_,
      scene_config_.physics_engine_config,
      &fifo_log_},
  primary_rendering_context_{RenderingContextStack::primary_resource_context()},
  secondary_rendering_context_{RenderingContextStack::resource_context()},
  primary_audio_resource_context_{AudioResourceContextStack::primary_resource_context()},
  secondary_audio_resource_context_{AudioResourceContextStack::resource_context()}
{
    if (config.with_flying_logic) {
        render_logics_.append(nullptr, flying_camera_logic_);
    }
    if (config.with_dirtmap) {
        render_logics_.append(nullptr, dirtmap_logic_);
    }
    render_logics_.append(nullptr, fxaa_logic_);
    physics_engine_.add_external_force_provider(&gefp_);
    physics_engine_.add_external_force_provider(key_bindings_.get());
}

RenderableScene::~RenderableScene() {
    RenderingContextGuard rrg0{primary_rendering_context_};
    RenderingContextGuard rrg1{secondary_rendering_context_};
    stop_and_join();
    std::lock_guard lock{ delete_node_mutex_ };
    scene_.shutdown();
    if (audio_listener_updater_ != nullptr) {
        physics_engine_.advance_times_.delete_advance_time(audio_listener_updater_.get());
    }
}

void RenderableScene::start_physics_loop(const std::string& thread_name) {
    if (physics_loop_ != nullptr) {
        throw std::runtime_error("physics loop already started");
    }
    physics_loop_.reset(
        new PhysicsLoop{
            thread_name,
            physics_iteration_,
            scene_config_.physics_engine_config,
            physics_set_fps_,
            SIZE_MAX,  // nframes
            RenderingContextStack::generate_thread_runner(
                primary_rendering_context_,
                secondary_rendering_context_)});
}

void RenderableScene::print_physics_engine_search_time() const {
    physics_engine_.rigid_bodies_.print_search_time();
}

void RenderableScene::plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    physics_engine_.rigid_bodies_.plot_triangle_bvh_svg(filename, axis0, axis1);
}

void RenderableScene::stop_and_join() {
    if (physics_loop_ != nullptr) {
        physics_loop_->stop_and_join();
    }
}

void RenderableScene::instantiate_audio_listener() {
    audio_listener_updater_ = std::make_unique<AudioListenerUpdater>(selected_cameras_, scene_);
    physics_engine_.advance_times_.add_advance_time(audio_listener_updater_.get());
}
