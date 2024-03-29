#include "Renderable_Scene.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Trail_Renderer.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fxaa_Logic.hpp>
#include <Mlib/Render/Render_Logics/Motion_Interpolation_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/Audio/Audio_Listener_Updater.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>

using namespace Mlib;

RenderableScene::RenderableScene(
    std::string rendering_resources_name,
    unsigned int max_anisotropic_filtering_level,
    SceneNodeResources& scene_node_resources,
    ParticleResources& particle_resources,
    TrailResources& trail_resources,
    SurfaceContactDb& surface_contact_db,
    BulletPropertyDb& bullet_property_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
#ifndef __ANDROID__
    GLFWwindow& glfw_window,
#endif
    const SceneConfigResource& config,
    const std::string& level_name,
    size_t max_tracks,
    bool save_playback,
    const RaceIdentifier& race_identfier,
    const std::function<void()>& setup_new_round,
    const FocusFilter& focus_filter,
    DependentSleeper& dependent_sleeper)
    : scene_node_resources_{ scene_node_resources }
    , particle_resources_{ particle_resources }
    , rendering_resources_{
        rendering_resources_name,
        max_anisotropic_filtering_level }
    , particle_renderer_{ std::make_unique<ParticleRenderer>(particle_resources) }
    , trail_renderer_{ std::make_unique<TrailRenderer>(trail_resources) }
    , scene_config_{ scene_config }
  // SceneNode destructors require that physics engine is destroyed after scene,
  // => Create PhysicsEngine before Scene
    , physics_engine_{ scene_config.physics_engine_config }
    , scene_{
          delete_node_mutex_,
          &scene_node_resources,
          particle_renderer_.get(),
          trail_renderer_.get()}
    , selected_cameras_{ scene_ }
    , user_object_{
          .button_states = button_states,
          .cursor_states = cursor_states,
          .scroll_wheel_states = scroll_wheel_states,
          .cameras = selected_cameras_,
          .wire_frame = scene_config.render_config.wire_frame,
          .depth_test = scene_config.render_config.depth_test,
          .cull_faces = scene_config.render_config.cull_faces,
          .delete_node_mutex = delete_node_mutex_,
          .physics_set_fps = &physics_set_fps_}
    , smoke_particle_generator_{ &rendering_resources_, scene_node_resources, scene_ }
    , contact_smoke_generator_{ surface_contact_db, smoke_particle_generator_ }
    , paused_{[&ui_focus, focus_filter](){
        std::shared_lock lock{ui_focus.focuses.mutex};
        return !ui_focus.has_focus(focus_filter);
      }}
    , physics_sleeper_{
          "Physics FPS: ",
          scene_config_.physics_engine_config.dt / s,
          scene_config_.physics_engine_config.max_residual_time / s,
          scene_config_.physics_engine_config.print_residual_time}
    , physics_set_fps_{
          scene_config_.physics_engine_config.control_fps
              ? &physics_sleeper_
              : nullptr,
          scene_config_.physics_engine_config.control_fps
              ? [this]() { return physics_sleeper_.simulated_time(); }
              : std::function<std::chrono::steady_clock::time_point()>(),
          paused_}
    , busy_state_provider_guard_{ dependent_sleeper, physics_set_fps_ }
    , gefp_{ gravity_vector }
    , physics_iteration_{
          scene_node_resources,
          scene_,
          physics_engine_,
          delete_node_mutex_,
          scene_config_.physics_engine_config,
          &fifo_log_}
    , standard_camera_logic_{
          scene_,
          selected_cameras_,
          delete_node_mutex_}
    , skybox_logic_{ standard_camera_logic_ }
    , standard_render_logic_{std::make_shared<StandardRenderLogic>(
          rendering_resources_,
          scene_,
          config.with_skybox
            ? (RenderLogic&)skybox_logic_
            : (RenderLogic&)standard_camera_logic_,
          config.background_color,
          config.clear_mode)}
    , flying_camera_logic_{config.with_flying_logic
          ? std::make_shared<FlyingCameraLogic>(
            scene_,
            user_object_,
            config.fly,
            config.rotate)
          : nullptr}
    , key_bindings_{std::make_shared<KeyBindings>(
          selected_cameras_,
          ui_focus.focuses,
          players_)}
    , read_pixels_logic_{ *standard_render_logic_ }
    , dirtmap_logic_{ std::make_shared<DirtmapLogic>(rendering_resources_, read_pixels_logic_) }
    , motion_interp_logic_{ std::make_shared<MotionInterpolationLogic>(read_pixels_logic_, InterpolationType::OPTICAL_FLOW) }
    , post_processing_logic_{std::make_shared<PostProcessingLogic>(
          *motion_interp_logic_,
          config.background_color,
          config.depth_fog,
          config.low_pass,
          config.high_pass)}
    , fxaa_logic_{ std::make_shared<FxaaLogic>(*post_processing_logic_) }
    , imposter_render_logics_{ std::make_shared<RenderLogics>(ui_focus) }
    , render_logics_{ ui_focus }
    , imposters_{ rendering_resources_, *imposter_render_logics_, read_pixels_logic_, scene_, selected_cameras_ }
    , players_{ physics_engine_.advance_times_, level_name, max_tracks, save_playback, scene_node_resources, race_identfier }
    , supply_depots_{ physics_engine_.advance_times_, players_, scene_config.physics_engine_config }
    , game_logic_{
          scene_,
          physics_engine_.advance_times_,
          vehicle_spawners_,
          players_,
          supply_depots_,
          delete_node_mutex_,
          setup_new_round}
#ifndef WITHOUT_ALUT
    , primary_audio_resource_context_{AudioResourceContextStack::primary_resource_context()}
#endif
{
    physics_engine_.set_contact_smoke_generator(contact_smoke_generator_);
    physics_engine_.set_particle_renderer(*particle_renderer_);
    physics_engine_.set_trail_renderer(*trail_renderer_);
    if (config.with_flying_logic) {
        render_logics_.append(nullptr, flying_camera_logic_, 0 /* z_order */);
    }
    render_logics_.append(nullptr, key_bindings_, 0 /* z_order */);
    render_logics_.append(nullptr, dirtmap_logic_, 0 /* z_order */);
    render_logics_.append(nullptr, imposter_render_logics_, 0 /* z_order */);
    render_logics_.append(nullptr, fxaa_logic_, 0 /* z_order */);
    physics_engine_.add_external_force_provider(gefp_);
    physics_engine_.add_external_force_provider(*key_bindings_);
}

RenderableScene::~RenderableScene() {
    stop_and_join();
    clear();
}

// RenderLogic
void RenderableScene::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::scoped_lock lock{delete_node_mutex_};
    DestructionGuard dg{[&](){standard_camera_logic_.reset();}};
    render_logics_.render(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

void RenderableScene::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderableScene\n";
    render_logics_.print(ostr, depth + 1);
}

// Misc
void RenderableScene::start_physics_loop(
    const std::string& thread_name,
    ThreadAffinity thread_affinity)
{
    if (physics_loop_ != nullptr) {
        THROW_OR_ABORT("physics loop already started");
    }
    physics_loop_.reset(
        new PhysicsLoop{
            thread_name,
            thread_affinity,
            physics_iteration_,
            physics_set_fps_,
            SIZE_MAX });  // nframes
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
        physics_loop_ = nullptr;
    }
    scene_.stop_and_join();
}

void RenderableScene::clear() {
    std::scoped_lock lock{ delete_node_mutex_ };
    scene_.shutdown();
    if (audio_listener_updater_ != nullptr) {
        physics_engine_.advance_times_.delete_advance_time(*audio_listener_updater_, CURRENT_SOURCE_LOCATION);
        audio_listener_updater_ = nullptr;
    }
}

void RenderableScene::instantiate_audio_listener(
    std::chrono::steady_clock::duration delay,
    std::chrono::steady_clock::duration velocity_dt)
{
    audio_listener_updater_ = std::make_unique<AudioListenerUpdater>(
        selected_cameras_,
        scene_,
        delay,
        velocity_dt);
    physics_engine_.advance_times_.add_advance_time(*audio_listener_updater_);
}
