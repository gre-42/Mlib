#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Audio/Audio_Scene.hpp>
#endif
#include <Mlib/Android/game_helper/AContext.hpp>
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Android/game_helper/ARenderLoop.hpp>
#include <Mlib/Android/game_helper/AWindow.hpp>
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <Mlib/Android/ndk_helper/AndroidApp.hpp>
#include <Mlib/Android/ndk_helper/NDKHelper.h>
#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translators.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Light_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Pretty_Terminate.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/IRenderer.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Descriptions.hpp>
#include <Mlib/Render/Print_Gl_Version_Info.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Menu_Logic.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Strings/Iterate_Over_Chunks_Of_String.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

class SceneRenderer: public IRenderer {
public:
    SceneRenderer(
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const ParsedArgs &args,
        RealtimeDependentFps& render_set_fps,
        MenuLogic& menu_logic)
    : render_config_{render_config},
      scene_graph_config_{scene_graph_config},
      render_results_{render_results},
      args_{args},
      render_set_fps_{render_set_fps},
      menu_logic_{menu_logic},
      renderable_scenes_{nullptr},
      load_scene_finished_{nullptr},
      last_load_scene_finished_{false}
    {
        render_set_fps_.set_fps.tick(std::chrono::steady_clock::time_point());
    }

    void load_resources() override {
        print_gl_version_info();
        // window_.set_frame_rate_if_supported(1.f / render_config_.min_dt);
    }
    void unload_resources() override {
        render_deallocator.deallocate();
    }
    void render(
        RenderEvent event,
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly) override
    {
        menu_logic_.handle_events();
        execute_render_gc();
        if (event != RenderEvent::LOOP) {
            return;
        }
        if (load_scene_finished_ == nullptr) {
            verbose_abort("load_scene_finished is null");
        }
        if (renderable_scenes_ == nullptr) {
            verbose_abort("renderable_scenes is null");
        }
        ViewportGuard vg{ lx.ilength(), ly.ilength() };
        auto rsd = rrsd_.next(render_config_.motion_interpolation, render_set_fps_.ft.frame_time());
        if (*load_scene_finished_) {
            execute_render_allocators();
            auto& rs = (*renderable_scenes_)["primary_scene"];
            rs.scene_.wait_for_cleanup();
            if (!last_load_scene_finished_ &&
                !args_.has_named("--no_physics") &&
                !args_.has_named("--single_threaded"))
            {
                for (auto& [n, r] : renderable_scenes_->guarded_iterable()) {
                    r.delete_node_mutex_.clear_deleter_thread();
                    r.start_physics_loop(("Phys_" + n).substr(0, 15), ThreadAffinity::POOL);
                }
                last_load_scene_finished_ = true;
            }
            rs.render_toplevel(
                lx,
                ly,
                render_config_,
                scene_graph_config_,
                render_results_,
                rsd);
            if (args_.has_named("--single_threaded")) {
                for (auto& [_, r] : renderable_scenes_->guarded_iterable()) {
                    SetDeleterThreadGuard set_deleter_thread_guard{ r.scene_.delete_node_mutex() };
                    if (!r.physics_set_fps_.paused()) {
                        r.physics_iteration_(std::chrono::steady_clock::now());
                    }
                    r.physics_set_fps_.execute_oldest_funcs();
                }
            }
        } else if (auto rs = renderable_scenes_->try_get("loading"); rs != nullptr) {
            execute_render_allocators();
            rs->scene_.wait_for_cleanup();
            if (rs->selected_cameras_.camera_node_exists()) {
                rs->render_toplevel(
                    lx,
                    ly,
                    render_config_,
                    scene_graph_config_,
                    render_results_,
                    rsd);
            }
        } else {
            clear_color({0.2f, 0.2f, 0.2f, 1.f});
        }
        render_set_fps_.set_fps.tick(rsd.external_render_pass.time);
    }

    void set_scene(
        RenderableScenes* renderable_scenes,
        std::atomic_bool* load_scene_finished)
    {
        renderable_scenes_ = renderable_scenes;
        load_scene_finished_ = load_scene_finished;
        last_load_scene_finished_ = false;
    }

private:
    const RenderConfig& render_config_;
    const SceneGraphConfig& scene_graph_config_;
    RenderResults* render_results_;
    const ParsedArgs &args_;
    RealtimeDependentFps& render_set_fps_;
    MenuLogic& menu_logic_;
    RenderableScenes* renderable_scenes_;
    std::atomic_bool* load_scene_finished_;
    RootRenderedSceneDescriptor rrsd_;
    bool last_load_scene_finished_;
};

void print_debug_info(
    const ParsedArgs& args,
    RenderableScenes& renderable_scenes)
{
    if (args.has_named("--print_search_time") ||
        args.has_named("--print_compression_ratio") ||
        args.has_named("--optimize_search_time") ||
        args.has_named("--plot_triangle_bvh"))
    {
        for (const auto& [n, r] : renderable_scenes.guarded_iterable()) {
            if (args.has_named("--print_search_time")) {
                linfo() << n << " search time";
                r.print_physics_engine_search_time();
            }
            if (args.has_named("--optimize_search_time")) {
                r.physics_engine_.rigid_bodies_.optimize_search_time(lraw().ref());
            }
            if (args.has_named("--plot_triangle_bvh")) {
                r.plot_physics_triangle_bvh_svg(n + "_xz.svg", 0, 2);
                r.plot_physics_triangle_bvh_svg(n + "_xy.svg", 0, 1);
            }
            if (args.has_named("--print_compression_ratio")) {
                r.physics_engine_.rigid_bodies_.print_compression_ratio();
            }
        }
    }
}

JThread loader_thread(
    const ParsedArgs& args,
    RenderLogicGallery& gallery,
    AssetReferences& asset_references,
    Translators& translators,
    RenderableScenes& renderable_scenes,
    const std::list<std::string>& search_path,
    const std::string& main_scene_filename,
    ThreadSafeString& next_scene_filename,
    NotifyingJsonMacroArguments& external_json_macro_arguments,
    std::atomic_size_t& num_renderings,
    RealtimeDependentFps& render_set_fps,
    SurfaceContactDb& surface_contact_db,
    BulletPropertyDb& bullet_property_db,
    DynamicLightDb& dynamic_light_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    ButtonPress& confirm_button_press,
    LockableKeyConfigurations& key_configurations,
    LockableKeyDescriptions& key_descriptions,
    UiFocus& ui_focus,
    LayoutConstraints& layout_constraints,
    LoadScene& load_scene,
    std::atomic_bool& load_scene_finished,
    std::chrono::steady_clock::duration render_delay,
    std::chrono::steady_clock::duration velocity_dt,
    const std::function<void()>& exit)
{
    return JThread{[&, render_delay, velocity_dt](){
        try {
            ThreadInitializer ti{"Scene loader", ThreadAffinity::POOL};
#ifndef WITHOUT_ALUT
            AudioResourceContext arc;
#endif
            {
#ifndef WITHOUT_ALUT
                AudioResourceContextGuard arcg{ arc };
                AudioListener::set_gain(safe_stof(args.named_value("--audio_gain", "1")));
#endif
                // GlContextGuard gcg{ render2.window() };
                load_scene(
                    &search_path,
                    main_scene_filename,
                    next_scene_filename,
                    external_json_macro_arguments,
                    num_renderings,
                    render_set_fps,
                    args.has_named("--verbose"),
                    surface_contact_db,
                    bullet_property_db,
                    dynamic_light_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    confirm_button_press,
                    key_configurations,
                    key_descriptions,
                    ui_focus,
                    layout_constraints,
                    gallery,
                    asset_references,
                    translators,
                    renderable_scenes,
                    exit);
                if (!args.has_named("--no_physics")) {
                    if (args.has_named("--no_render")) {
                        for (auto& [n, r] : renderable_scenes.guarded_iterable()) {
                            r.delete_node_mutex_.clear_deleter_thread();
                            r.start_physics_loop(("Phys_" + n).substr(0, 15), ThreadAffinity::POOL);
                        }
                    } else if (args.has_named("--single_threaded")) {
                        for (auto& [n, r] : renderable_scenes.guarded_iterable()) {
                            r.scene_.delete_node_mutex().clear_deleter_thread();
                        }
                    }
                }
                load_scene_finished = true;
                renderable_scenes["primary_scene"].instantiate_audio_listener(
                    render_delay,
                    velocity_dt);
            }

            print_debug_info(args, renderable_scenes);

        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    }};
}

void android_main(android_app* app) {
    // set_log_level(LogLevel::ERROR);
    AndroidAppGuard android_app_guard{*app};
    // This throws exceptions internally, which is not supported
    // on Android.
    // register_pretty_terminate();
    // This currently has no effect, because the implementation is empty on Android.
    // Enabling floating point exceptions did not work on Android.
    enable_floating_point_exceptions();
    // This code sometimes caused crashes on some devices,
    // it is now moved to the NdkTestActivity Java class.
    // AUi::SetRequestedScreenOrientation(ScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE);

    // This currently has no effect, because the implementation is empty on Android.
    // The render-loop is called by the system, which is therefore in control of the frame rate.
    RealtimeThreadsGuard rttsg{ 0 };
    ThreadInitializer ti{"Main", ThreadAffinity::POOL};

    const ArgParser parser(
        "Usage: render_scene_file working_directory scene.scn.json\n"
        "    [--wire_frame]\n"
        "    [--cull_faces]\n"
        "    [--fly]\n"
        "    [--rotate]\n"
        "    [--swap_interval <interval>]\n"
        "    [--nsamples_msaa <nsamples>]\n"
        "    [--lightmap_nsamples_msaa <nsamples>]\n"
        "    [--min_sample_shading <rate>]\n"
        "    [--fxaa]\n"
        "    [--max_distance_black <distance>]\n"
        "    [--small_aggregate_update_interval <interval>]\n"
        "    [--large_max_offset_deviation <interval>]\n"
        "    [--windowed_width <width>]\n"
        "    [--windowed_height <height>]\n"
        "    [--fullscreen_width <width>]\n"
        "    [--fullscreen_height <height>]\n"
        "    [--scene_lightmap_width <width>]\n"
        "    [--scene_lightmap_height <height>]\n"
        "    [--black_lightmap_width <width>]\n"
        "    [--black_lightmap_height <height>]\n"
        "    [--scene_skidmarks_width <width>]\n"
        "    [--scene_skidmarks_height <height>]\n"
        "    [--fullscreen]\n"
        "    [--no_double_buffer]\n"
        "    [--anisotropic_filtering_level <value>]\n"
        "    [--no_normalmaps]\n"
        "    [--no_physics]\n"
        "    [--physics_dt <dt>]\n"
        "    [--render_dt <dt>]\n"
        "    [--render_max_residual_time <dt>]\n"
        "    [--no_control_physics_fps ]\n"
        "    [--control_render_fps]\n"
        "    [--print_physics_residual_time]\n"
        "    [--print_render_residual_time]\n"
        "    [--draw_distance_add <value>]\n"
        "    [--far_plane <value>]\n"
        "    [--record_track_basename <value>]\n"
        "    [--devel_mode]\n"
        "    [--enable_ridge_map]\n"
        "    [--stiction_coefficient <x>]\n"
        "    [--friction_coefficient <x>]\n"
        "    [--max_extra_friction <x>]\n"
        "    [--max_extra_w <x>]\n"
        "    [--longitudinal_friction_steepness <x>]\n"
        "    [--lateral_friction_steepness <x>]\n"
        "    [--no_slip <x>]\n"
        "    [--no_avoid_burnout]\n"
        "    [--wheel_penetration_depth <x>]\n"
        "    [--vfx]\n"
        "    [--no_depth_fog]\n"
        "    [--low_pass]\n"
        "    [--high_pass]\n"
        "    [--bloom_x <niterations>]\n"
        "    [--bloom_y <niterations>]\n"
        "    [--bloom_thresold <threshold>]\n"
        "    [--motion_interpolation]\n"
        "    [--no_render]\n"
        "    [--save_playback]\n"
        "    [--optimize_search_time]\n"
        "    [--plot_triangle_bvh]\n"
        "    [--show_mouse_cursor]\n"
        "    [--nsubsteps <n>]\n"
        "    [--bvh_max_size <r>]\n"
        "    [--static_radius <r>]\n"
        "    [--print_search_time]\n"
        "    [--print_compression_ratio]\n"
        "    [--num_renderings <n>]\n"
        "    [--audio_gain <f>]\n"
        "    [--show_debug_wheels]\n"
        "    [--write_loaded_resources <dir>]\n"
        "    [--audio_frequency <value>]\n"
        "    [--audio_alpha <value>]\n"
        "    [--check_gl_errors]\n"
        "    [--verbose]",
        {"--wire_frame",
         "--cull_faces",
         "--fly",
         "--rotate",
         "--no_physics",
         "--fullscreen",
         "--no_double_buffer",
         "--no_normalmaps",
         "--print_physics_residual_time",
         "--print_render_residual_time",
         "--single_threaded",
         "--vfx",
         "--no_depth_fog",
         "--low_pass",
         "--high_pass",
         "--motion_interpolation",
         "--no_render",
         "--save_playback",
         "--optimize_search_time",
         "--plot_triangle_bvh",
         "--devel_mode",
         "--show_mouse_cursor",
         "--enable_ridge_map",
         "--no_slip",
         "--no_avoid_burnout",
         "--print_search_time",
         "--print_compression_ratio",
         "--no_control_physics_fps",
         "--control_render_fps",
         "--fxaa",
         "--check_gl_errors",
         "--verbose"},
        {"--record_track_basename",
         "--swap_interval",
         "--nsamples_msaa",
         "--lightmap_nsamples_msaa",
         "--min_sample_shading",
         "--anisotropic_filtering_level",
         "--max_distance_black",
         "--small_aggregate_update_interval",
         "--large_aggregate_update_interval",
         "--print_render_fps_interval",
         "--windowed_width",
         "--windowed_height",
         "--fullscreen_width",
         "--fullscreen_height",
         "--scene_lightmap_width",
         "--scene_lightmap_height",
         "--black_lightmap_width",
         "--black_lightmap_height",
         "--scene_skidmarks_width",
         "--scene_skidmarks_height",
         "--static_radius",
         "--bvh_max_size",
         "--physics_dt",
         "--nsubsteps",
         "--render_dt",
         "--render_max_residual_time",
         "--stiction_coefficient",
         "--friction_coefficient",
         "--max_extra_w",
         "--max_extra_friction",
         "--longitudinal_friction_steepness",
         "--lateral_friction_steepness",
         "--wheel_penetration_depth",
         "--draw_distance_add",
         "--far_plane",
         "--num_renderings",
         "--audio_gain",
         "--show_debug_wheels",
         "--write_loaded_resources",
         "--audio_frequency",
         "--audio_alpha",
         "--bloom_x",
         "--bloom_y",
         "--bloom_threshold"});
    try {
        const char* argv[] = {"appname", "/;/data", "/levels/main/main.scn.json"};
        const auto args = parser.parsed(sizeof(argv) / sizeof(argv[0]), argv);

        args.assert_num_unnamed(2);
        std::list<std::string> search_path = string_to_list(args.unnamed_value(0), Mlib::compile_regex(";"));
        std::string main_scene_filename = fs::absolute(args.unnamed_value(1)).string();

        if (args.has_named("--check_gl_errors")) {
            check_gl_errors(CheckErrors::ENABLED);
        }
#ifndef WITHOUT_ALUT
        AudioDevice audio_device;
        AudioContext audio_context{audio_device, safe_stou(args.named_value("--audio_frequency", "0"))};
        linfo() << "Audio frequency: " << audio_device.get_frequency();
        AudioScene::set_default_alpha(safe_stof(args.named_value("--audio_alpha", "0.1")));
#endif

        std::atomic_size_t num_renderings;
        RenderConfig render_config{
            .nsamples_msaa = safe_stoi(args.named_value("--nsamples_msaa", "1")),
            .lightmap_nsamples_msaa = safe_stoi(args.named_value("--lightmap_nsamples_msaa", "1")),
            .min_sample_shading = safe_stof(args.named_value("--min_sample_shading", "0")),
            .vfx = args.has_named("--vfx"),
            .fxaa = args.has_named("--fxaa"),
            .cull_faces = args.has_named("--cull_faces")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .wire_frame = args.has_named("--wire_frame")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .window_title = main_scene_filename,
            .windowed_width = safe_stoi(args.named_value("--windowed_width", "800")),
            .windowed_height = safe_stoi(args.named_value("--windowed_height", "600")),
            .fullscreen_width = safe_stoi(args.named_value("--fullscreen_width", "0")),
            .fullscreen_height = safe_stoi(args.named_value("--fullscreen_height", "0")),
            .motion_interpolation = args.has_named("--motion_interpolation"),
            .fullscreen = args.has_named("--fullscreen"),
            .double_buffer = !args.has_named("--no_double_buffer"),
            .anisotropic_filtering_level = safe_stou(args.named_value("--anisotropic_filtering_level", "0")),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = args.has_named("--show_mouse_cursor"),
            .swap_interval = safe_stoi(args.named_value("--swap_interval", "1")),
            .fullscreen_refresh_rate = safe_stoi(args.named_value("--fullscreen_refresh_rate", "0")),
            .draw_distance_add = safe_stof(args.named_value("--draw_distance_add", "inf"))};
        auto physics_dt = safe_stof(args.named_value("--physics_dt", "0.01667"));
        auto render_delay = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>{ 1.0f * physics_dt });
        auto velocity_dt = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>{ 0.1f * physics_dt });
        RealtimeDependentFps render_set_fps{
            "Render set FPS: ",
            safe_stof(args.named_value("--render_dt", "0.01667")),
            render_delay,
            safe_stof(args.named_value("--render_max_residual_time", "0.5")),
            args.has_named("--control_render_fps"),
            args.has_named("--print_render_residual_time"),
            0.05f,
            0.05f,
            safe_stou(args.named_value("--print_render_fps_interval", "-1"))};

        SceneGraphConfig scene_graph_config{
            .max_distance_black = safe_stof(args.named_value("--max_distance_black", "200")) * meters,
            .small_aggregate_update_interval = safe_stoz(args.named_value("--small_aggregate_update_interval", "60")),
            .large_max_offset_deviation = safe_stof(args.named_value("--large_max_offset_deviation", "200")) * meters};

        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        BaseKeyCombination confirm_key_combination{{{
            BaseKeyBinding{
                .key = "ENTER",
                .gamepad_button = "A",
                .tap_button = "START"}}}};
        LockableKeyConfigurations confirm_key_configurations;
        confirm_key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->emplace()
            .insert("confirm", { std::move(confirm_key_combination) });
        ButtonPress confirm_button_press{ button_states, confirm_key_configurations, "confirm", "" };
        UiFocus ui_focus{ get_path_in_appdata_directory({"focus.json"}) };
        if (ui_focus.can_load()) {
            ui_focus.load();
        }
        // AWindow window{*app};
        MenuUserClass menu_user_object{
            .button_states = button_states,
            .focuses = ui_focus.focuses};
        MenuLogic menu_logic{menu_user_object};
        // Declared as first class to let destructors of other classes succeed.
        SceneRenderer scene_renderer{
            render_config,
            scene_graph_config,
            nullptr,    // render_results
            args,
            render_set_fps,
            menu_logic};
        AEngine a_engine{ scene_renderer, button_states };
        AContext context;
        ContextQueryGuard context_query_guard{ context };
        ClearWrapperGuard clear_wrapper_guard;
        ARenderLoop render_loop{ *app, a_engine };
        // AUi::RequestReadExternalStoragePermission();

        NotifyingJsonMacroArguments external_json_macro_arguments;
        // FifoLog fifo_log{10 * 1000};

        size_t args_num_renderings = safe_stoz(args.named_value("--num_renderings", "-1"));
        while (!render_loop.destroy_requested() && !unhandled_exceptions_occured()) {
            num_renderings = args_num_renderings;
            ui_focus.clear();
            button_states.tap_buttons_.clear();

            PhysicsEngineConfig physics_engine_config{
                .dt = physics_dt * seconds,
                .control_fps = !args.has_named("--no_control_physics_fps"),
                .print_residual_time = args.has_named("--print_physics_residual_time"),
                // BVH
                .static_radius = (CompressedScenePos)(safe_stof(args.named_value("--static_radius", "20")) * meters),
                .bvh_max_size = (CompressedScenePos)(safe_stof(args.named_value("--bvh_max_size", "2")) * meters),
                // Collision/Friction misc.
                .max_extra_friction = safe_stof(args.named_value("--max_extra_friction", "0")),
                .max_extra_w = safe_stof(args.named_value("--max_extra_w", "0")),
                .avoid_burnout = !args.has_named("--no_avoid_burnout"),
                .no_slip = args.has_named("--no_slip"),
                // Friction
                .stiction_coefficient = safe_stof(args.named_value("--stiction_coefficient", "0.5")),
                .friction_coefficient = safe_stof(args.named_value("--friction_coefficient", "0.5")),
                .longitudinal_friction_steepness = safe_stof(args.named_value("--longitudinal_friction_steepness", "5")),
                .lateral_friction_steepness = safe_stof(args.named_value("--lateral_friction_steepness", "7")),
                // Collision
                .wheel_penetration_depth = safe_stof(args.named_value("--wheel_penetration_depth", "0.25")),
                .nsubsteps = safe_stoz(args.named_value("--nsubsteps", "8")),
                .enable_ridge_map = args.has_named("--enable_ridge_map")};

            SceneConfig scene_config{
                .render_config = render_config,
                .scene_graph_config = scene_graph_config,
                .physics_engine_config = physics_engine_config};

            SceneNodeResources scene_node_resources;
            ParticleResources particle_resources;
            TrailResources trail_resources;
            SurfaceContactDb surface_contact_db;
            BulletPropertyDb bullet_property_db;
            DynamicLightDb dynamic_light_db;
            LayoutConstraints layout_constraints;
            LockableKeyConfigurations key_configurations;
            LockableKeyDescriptions key_descriptions;
            {
                auto record_track_basename = args.try_named_value("--record_track_basename");
                nlohmann::json j{
                    {"primary_scene_fly", args.has_named("--fly")},
                    {"primary_scene_rotate", args.has_named("--rotate")},
                    {"primary_scene_depth_fog", !args.has_named("--no_depth_fog")},
                    {"primary_scene_low_pass", args.has_named("--low_pass")},
                    {"primary_scene_high_pass", args.has_named("--high_pass")},
                    {"primary_scene_bloom_iterations", FixedArray<unsigned int, 2>{
                        safe_stou(args.named_value("--bloom_x", "3")),
                        safe_stou(args.named_value("--bloom_y", "3"))}},
                    {"primary_scene_bloom_thresholds", fixed_full<float, 3>(
                        safe_stof(args.named_value("--bloom_threshold", "1")))},
                    {"primary_scene_with_skybox", true},
                    {"primary_scene_with_flying_logic", true},
                    {"primary_scene_save_playback", args.has_named("--save_playback")},
                    {"far_plane", safe_stof(args.named_value("--far_plane", "10000"))},
                    {"record_track_basename", (record_track_basename == nullptr)
                        ? nlohmann::json()
                        : nlohmann::json(*record_track_basename)},
                    {"if_devel", args.has_named("--devel_mode")},
                    {"if_show_debug_wheels", args.has_named("--show_debug_wheels")},
                    {"if_android", true},
                    {"flavor", AUi::GetFlavor()},
                    {"scene_lightmap_width", safe_stoi(args.named_value("--scene_lightmap_width", "2048"))},
                    {"scene_lightmap_height", safe_stoi(args.named_value("--scene_lightmap_height", "2048"))},
                    {"black_lightmap_width", safe_stoi(args.named_value("--black_lightmap_width", "1024"))},
                    {"black_lightmap_height", safe_stoi(args.named_value("--black_lightmap_height", "1024"))},
                    {"scene_skidmarks_width", safe_stoi(args.named_value("--scene_skidmarks_width", "2048"))},
                    {"scene_skidmarks_height", safe_stoi(args.named_value("--scene_skidmarks_height", "2048"))}};
                external_json_macro_arguments.merge_and_notify(JsonMacroArguments{std::move(j)});
            }
            // "load_scene" must be above "renderable_scenes", because the "RenderableScene" background
            // threads have lambda functions operating on the "load_scene.macro_recorder_" object.
            // In case of an exception in the main thread, destruction of "load_scene" must therefore happen
            // after the destruction of "renderable_scenes".
            LoadScene load_scene;
            ThreadSafeString next_scene_filename;
            {
                RenderingResources rendering_resources{
                    "primary_rendering_resources",
                    render_config.anisotropic_filtering_level
                };
                rendering_resources.add_charset(ascii, ascii_chars());
                RenderingContext primary_rendering_context{
                    .scene_node_resources = scene_node_resources,
                    .particle_resources = particle_resources,
                    .trail_resources = trail_resources,
                    .rendering_resources = rendering_resources,
                    .z_order = 0
                };
                RenderingContextGuard rcg{ primary_rendering_context };

                RenderLogicGallery gallery;
                AssetReferences asset_references;
                Translators translators{ asset_references, external_json_macro_arguments };
                RenderableScenes renderable_scenes;

                std::atomic_bool load_scene_finished = false;
                scene_renderer.set_scene(&renderable_scenes, &load_scene_finished);
                DestructionGuard dg0{[&scene_renderer](){ scene_renderer.set_scene(nullptr, nullptr); }};

                DestructionGuard dg1{[](){discard_render_allocators();}};
                std::function<void()> exit = [](){
                    lerr() << "Program exit not supported on Android";
                };
                JThread loader_future_guard{loader_thread(
                    args,
                    gallery,
                    asset_references,
                    translators,
                    renderable_scenes,
                    search_path,
                    main_scene_filename,
                    next_scene_filename,
                    external_json_macro_arguments,
                    num_renderings,
                    render_set_fps,
                    surface_contact_db,
                    bullet_property_db,
                    dynamic_light_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    confirm_button_press,
                    key_configurations,
                    key_descriptions,
                    ui_focus,
                    layout_constraints,
                    load_scene,
                    load_scene_finished,
                    render_delay,
                    velocity_dt,
                    exit)};
                render_loop.render_loop([&num_renderings](){return (num_renderings == 0) || unhandled_exceptions_occured();});
                if (args.has_named_value("--write_loaded_resources")) {
                    scene_node_resources.write_loaded_resources(args.named_value("--write_loaded_resources"));
                }
            }
            {
                std::scoped_lock lock{ui_focus.focuses.mutex};
                ui_focus.focuses.set_focuses({});
            }
            if (auto s = (std::string)next_scene_filename; !s.empty()) {
                main_scene_filename = s;
            }
        }

        if (ui_focus.has_changes() && ui_focus.can_save()) {
            ui_focus.save();
        }

        // if (!TimeGuard::is_empty(std::this_thread::get_id())) {
        //     lerr() << "write svg";
        //     TimeGuard::write_svg(std::this_thread::get_id(), "/tmp/events.svg");
        // }
    } catch (const CommandLineArgumentError& e) {
        LOGE("Command-line error: %s", e.what());
        AUi::ShowMessage("Error", e.what());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::abort();
    } catch (const std::runtime_error& e) {
        for (const auto substr : iterate_over_blocks_of_string(e.what(), 1000))
        {
            lerr() << "Runtime error: " << substr;
        }
        AUi::ShowMessage("Error", e.what());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::abort();
    }
    if (unhandled_exceptions_occured()) {
        std::stringstream sstr;
        print_unhandled_exceptions(sstr);
        for (const auto substr : iterate_over_blocks_of_string(sstr.str(), 1000))
        {
            lerr() << "Unhandled exception(s): " << substr;
        }
        AUi::ShowMessage("Error", sstr.str());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::abort();
    }
}
