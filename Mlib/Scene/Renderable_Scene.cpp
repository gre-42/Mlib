#include "Renderable_Scene.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Aggregate_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Bloom_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fxaa_Logic.hpp>
#include <Mlib/Render/Render_Logics/Motion_Interpolation_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene/Audio/Audio_Listener_Updater.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>

using namespace Mlib;

RenderableScene::RenderableScene(
    std::string name,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    LockableKeyConfigurations& key_configurations,
    UiFocus& ui_focus,
    const FocusFilter& focus_filter,
    uint32_t user_id,
    const SceneConfigResource& config)
    : on_stop_and_join_physics_{ physics_scene->on_stop_and_join_, CURRENT_SOURCE_LOCATION }
    , on_clear_physics_{ physics_scene->on_clear_, CURRENT_SOURCE_LOCATION }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
    , counter_user_{ DanglingBaseClassRef<UsageCounter>{physics_scene->usage_counter_, CURRENT_SOURCE_LOCATION} }
    , name_{ std::move(name) }
    , physics_scene_{ physics_scene.ptr() }
    , scene_config_{ scene_config }
    , selected_cameras_{ physics_scene->scene_ }
    , user_object_{
          .button_states = button_states,
          .cursor_states = cursor_states,
          .scroll_wheel_states = scroll_wheel_states,
          .cameras = selected_cameras_,
          .wire_frame = scene_config.render_config.wire_frame,
          .depth_test = scene_config.render_config.depth_test,
          .cull_faces = scene_config.render_config.cull_faces,
          .delete_node_mutex = physics_scene->delete_node_mutex_,
          .physics_set_fps = &physics_scene->physics_set_fps_}
    , ui_focus_{ ui_focus }
    , focus_filter_{ focus_filter }
    , user_id_{ user_id }
    , render_logics_{ ui_focus }
    , scene_render_logics_{ ui_focus }
    , standard_camera_logic_{
          physics_scene->scene_,
          selected_cameras_}
    , skybox_logic_{ standard_camera_logic_ }
    , standard_render_logic_{std::make_unique<StandardRenderLogic>(
          physics_scene->scene_,
          config.with_skybox
            ? (RenderLogic&)skybox_logic_
            : (RenderLogic&)standard_camera_logic_,
          config.background_color,
          config.clear_mode)}
    , aggregate_render_logic_{std::make_unique<AggregateRenderLogic>(
        physics_scene->rendering_resources_,
        *standard_render_logic_)}
    , flying_camera_logic_{config.with_flying_logic
          ? std::make_unique<FlyingCameraLogic>(
            physics_scene->scene_,
            user_object_,
            config.fly,
            config.rotate)
          : nullptr}
    , key_bindings_{std::make_unique<KeyBindings>(
          selected_cameras_,
          ui_focus.focuses,
          physics_scene->players_,
          physics_scene->physics_engine_)}
    , read_pixels_logic_{ *aggregate_render_logic_, button_states, key_configurations, ReadPixelsRole::INTERMEDIATE }
    , dirtmap_logic_{ std::make_unique<DirtmapLogic>(physics_scene->rendering_resources_, read_pixels_logic_) }
    , motion_interp_logic_{ std::make_unique<MotionInterpolationLogic>(read_pixels_logic_, InterpolationType::OPTICAL_FLOW) }
    , post_processing_logic_{std::make_unique<PostProcessingLogic>(
          *motion_interp_logic_,
          config.background_color,
          config.depth_fog,
          config.low_pass,
          config.high_pass)}
    , fxaa_logic_{ std::make_unique<FxaaLogic>(*post_processing_logic_) }
    , bloom_logic_{ std::make_unique<BloomLogic>(
        scene_render_logics_,
        config.bloom_thresholds,
        config.bloom_iterations) }
    , imposter_render_logics_{ std::make_unique<RenderLogics>(ui_focus) }
    , imposters_instantiated_{ false }
    , background_color_applied_{ false }
{
    render_logics_.append({ physics_scene->render_logics_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    if (config.with_flying_logic) {
        render_logics_.append({ *flying_camera_logic_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    }
    render_logics_.append({ *key_bindings_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *dirtmap_logic_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *imposter_render_logics_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *bloom_logic_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    scene_render_logics_.append({ *fxaa_logic_, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);

    on_stop_and_join_physics_.add([this](){
        aggregate_render_logic_->stop_and_join();
    }, CURRENT_SOURCE_LOCATION);
    on_clear_physics_.add([this](){
        if (audio_listener_updater_ != nullptr) {
            physics_scene_->physics_engine_.advance_times_.delete_advance_time(*audio_listener_updater_, CURRENT_SOURCE_LOCATION);
            audio_listener_updater_ = nullptr;
        }
        physics_scene_->physics_engine_.remove_external_force_provider(*key_bindings_);
        render_logics_.remove(physics_scene_->render_logics_);
        counter_user_.reset();
        physics_scene_ = nullptr;
    }, CURRENT_SOURCE_LOCATION);

    physics_scene_->physics_engine_.add_external_force_provider(*key_bindings_);
}

RenderableScene::~RenderableScene() {
    object_pool_.clear();
    on_clear_physics_.clear();
}

// RenderLogic
std::optional<RenderSetup> RenderableScene::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void RenderableScene::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (physics_scene_ == nullptr) {
        verbose_abort("RenderableScene::render_without_setup with null physics scene");
    }
    if (!imposters_instantiated_) {
        physics_scene_->deferred_instantiator_.create_imposters(
            this,
            physics_scene_->rendering_resources_,
            read_pixels_logic_,
            *imposter_render_logics_,
            physics_scene_->scene_,
            selected_cameras_);
        imposters_instantiated_ = true;
    }
    if (!background_color_applied_ &&
        physics_scene_->deferred_instantiator_.has_background_color())
    {
        physics_scene_->deferred_instantiator_.apply_background_color(
            *standard_render_logic_,
            *post_processing_logic_);
        background_color_applied_ = true;
    }
    {
        std::shared_lock lock{ ui_focus_.focuses.mutex };
        counter_user_->set(ui_focus_.has_focus(focus_filter_));
    }

    auto f = frame_id;
    f.external_render_pass.user_id = user_id_;
    auto completed_time = physics_scene_->physics_set_fps_.completed_time();
    if (completed_time != std::chrono::steady_clock::time_point()) {
        f.external_render_pass.time = std::min(
            f.external_render_pass.time,
            completed_time);
    }
    render_logics_.render_without_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        f);
}

void RenderableScene::wait_until_done() const {
    aggregate_render_logic_->wait_until_done();
}

void RenderableScene::stop_and_join() {
    // Do nothing
}

void RenderableScene::clear() {
    // Do nothing
}

void RenderableScene::instantiate_audio_listener(
    std::chrono::steady_clock::duration delay,
    std::chrono::steady_clock::duration velocity_dt)
{
    audio_listener_updater_ = std::make_unique<AudioListenerUpdater>(
        selected_cameras_,
        physics_scene_->scene_,
        delay,
        velocity_dt);
    physics_scene_->physics_engine_.advance_times_.add_advance_time(
        { *audio_listener_updater_, CURRENT_SOURCE_LOCATION },
        CURRENT_SOURCE_LOCATION);
}

void RenderableScene::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderableScene\n";
    render_logics_.print(ostr, depth + 1);
}
