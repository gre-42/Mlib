#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#endif
#include <Mlib/Android/game_helper/AContext.hpp>
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Android/game_helper/ARenderLoop.hpp>
#include <Mlib/Android/game_helper/AWindow.hpp>
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <Mlib/Android/ndk_helper/AndroidApp.hpp>
#include <Mlib/Android/ndk_helper/NDKHelper.h>
#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Pretty_Terminate.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/IRenderer.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Print_Gl_Version_Info.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Menu_Logic.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Strings/Iterate_Over_Chunks_Of_String.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Future_Guard.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>
#include <filesystem>
#include <future>

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
    {}

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
        if (*load_scene_finished_) {
            execute_render_allocators();
            if (!last_load_scene_finished_ &&
                !args_.has_named("--no_physics") &&
                !args_.has_named("--single_threaded"))
            {
                for (auto& [n, r] : renderable_scenes_->guarded_iterable()) {
                    r.delete_node_mutex_.clear_deleter_thread();
                    r.start_physics_loop(("Physics_" + n).substr(0, 15), ThreadAffinity::POOL);
                }
                last_load_scene_finished_ = true;
            }
            (*renderable_scenes_)["primary_scene"].render(
                lx,
                ly,
                render_config_,
                scene_graph_config_,
                render_results_,
                rrsd_.next(render_config_.motion_interpolation));
            if (args_.has_named("--single_threaded")) {
                for (auto& [_, r]: renderable_scenes_->guarded_iterable()) {
                    if (!r.physics_set_fps_.paused()) {
                        r.physics_iteration_(std::chrono::steady_clock::now());
                    }
                }
            }
        } else if (renderable_scenes_->contains("loading")) {
            auto &rs = (*renderable_scenes_)["loading"];
            std::scoped_lock lock{rs.scene_.delete_node_mutex()};
            if (rs.scene_.contains_node(rs.selected_cameras_.camera_node_name())) {
                rs.render(
                    lx,
                    ly,
                    render_config_,
                    scene_graph_config_,
                    render_results_,
                    rrsd_.next(render_config_.motion_interpolation));
            }
        } else {
            clear_color({0.2f, 0.2f, 0.2f, 1.f});
        }
        render_set_fps_.set_fps.tick();
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
        args.has_named("--optimize_search_time") ||
        args.has_named("--plot_triangle_bvh"))
    {
        for (const auto& [n, r] : renderable_scenes.guarded_iterable()) {
            if (args.has_named("--print_search_time")) {
                std::cerr << n << " search time" << std::endl;
            }
            r.print_physics_engine_search_time();
            if (args.has_named("--optimize_search_time")) {
                r.physics_engine_.rigid_bodies_.optimize_search_time(std::cerr);
            }
            if (args.has_named("--plot_triangle_bvh")) {
                r.plot_physics_triangle_bvh_svg(n + "_xz.svg", 0, 2);
                r.plot_physics_triangle_bvh_svg(n + "_xy.svg", 0, 1);
            }
        }
    }
}

std::future<void> loader_thread(
    const ParsedArgs& args,
    RenderLogicGallery& gallery,
    AssetReferences& asset_references,
    RenderableScenes& renderable_scenes,
    const std::list<std::string>& search_path,
    const std::string& main_scene_filename,
    ThreadSafeString& next_scene_filename,
    NotifyingJsonMacroArguments& external_substitutions,
    std::atomic_size_t& num_renderings,
    RealtimeDependentFps& render_set_fps,
    SurfaceContactDb& surface_contact_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    LayoutConstraints& layout_constraints,
    LoadScene& load_scene,
    std::atomic_bool& load_scene_finished)
{
    return std::async(std::launch::async, [&](){
        try {
            ThreadInitializer ti{"scene_loader", ThreadAffinity::POOL};
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
                    external_substitutions,
                    num_renderings,
                    render_set_fps,
                    args.has_named("--verbose"),
                    surface_contact_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    ui_focus,
                    layout_constraints,
                    gallery,
                    asset_references,
                    renderable_scenes);
                if (!args.has_named("--no_physics")) {
                    if (args.has_named("--no_render")) {
                        for (auto& [n, r] : renderable_scenes.guarded_iterable()) {
                            r.delete_node_mutex_.clear_deleter_thread();
                            r.start_physics_loop(("Physics_" + n).substr(0, 15), ThreadAffinity::POOL);
                        }
                    } else if (args.has_named("--single_threaded")) {
                        for (auto& [n, r] : renderable_scenes.guarded_iterable()) {
                            r.scene_.delete_node_mutex().clear_deleter_thread();
                        }
                    }
                }
                load_scene_finished = true;
                renderable_scenes["primary_scene"].instantiate_audio_listener();
            }

            print_debug_info(args, renderable_scenes);

        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    });
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
    reserve_realtime_threads(0);

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
        "    [--large_aggregate_update_interval <interval>]\n"
        "    [--windowed_width <width>]\n"
        "    [--windowed_height <height>]\n"
        "    [--fullscreen_width <width>]\n"
        "    [--fullscreen_height <height>]\n"
        "    [--scene_lightmap_width <width>]\n"
        "    [--scene_lightmap_height <height>]\n"
        "    [--black_lightmap_width <width>]\n"
        "    [--black_lightmap_height <height>]\n"
        "    [--fullscreen]\n"
        "    [--no_double_buffer]\n"
        "    [--anisotropic_filtering_level <value>]\n"
        "    [--no_normalmaps]\n"
        "    [--no_physics ]\n"
        "    [--physics_dt <dt> ]\n"
        "    [--render_dt <dt> ]\n"
        "    [--render_max_residual_time <dt> ]\n"
        "    [--no_control_physics_fps ]\n"
        "    [--control_render_fps ]\n"
        "    [--print_physics_residual_time]\n"
        "    [--print_render_residual_time]\n"
        "    [--draw_distance_add <value>]\n"
        "    [--far_plane <value>]\n"
        "    [--record_track]\n"
        "    [--devel_mode]\n"
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
        "    [--motion_interpolation]\n"
        "    [--no_render]\n"
        "    [--save_playback]\n"
        "    [--optimize_search_time]\n"
        "    [--plot_triangle_bvh]\n"
        "    [--print_gamepad_buttons]\n"
        "    [--show_mouse_cursor]\n"
        "    [--oversampling]\n"
        "    [--bvh_max_size <r>]\n"
        "    [--static_radius <r>]\n"
        "    [--print_search_time]\n"
        "    [--num_renderings <n>]\n"
        "    [--audio_gain <f>]\n"
        "    [--show_debug_wheels]\n"
        "    [--write_loaded_resources <dir>]\n"
        "    [--audio_frequency <value>]\n"
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
         "--record_track",
         "--devel_mode",
         "--print_gamepad_buttons",
         "--show_mouse_cursor",
         "--no_slip",
         "--no_avoid_burnout",
         "--print_search_time",
         "--no_control_physics_fps",
         "--control_render_fps",
         "--fxaa",
         "--verbose"},
        {"--swap_interval",
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
         "--static_radius",
         "--bvh_max_size",
         "--physics_dt",
         "--oversampling",
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
         "--audio_frequency"});
    try {
        const char* argv[] = {"appname", "/;/data", "/levels/main/main.scn.json"};
        const auto args = parser.parsed(3, argv);

        args.assert_num_unnamed(2);
        std::list<std::string> search_path = string_to_list(args.unnamed_value(0), Mlib::compile_regex(";"));
        std::string main_scene_filename = fs::absolute(args.unnamed_value(1)).string();

#ifndef WITHOUT_ALUT
        AudioDevice audio_device;
        AudioContext audio_context{audio_device, safe_stou(args.named_value("--audio_frequency", "0"))};
        linfo() << "Audio frequency: " << audio_device.get_frequency();
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
            .windowed_width = safe_stoi(args.named_value("--windowed_width", "640")),
            .windowed_height = safe_stoi(args.named_value("--windowed_height", "480")),
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
        RealtimeDependentFps render_set_fps{
            "Render set FPS: ",
            safe_stof(args.named_value("--render_dt", "0.01667")),
            physics_dt,
            safe_stof(args.named_value("--render_max_residual_time", "0.5")),
            args.has_named("--control_render_fps"),
            args.has_named("--print_render_residual_time"),
            0.05f,
            safe_stou(args.named_value("--print_render_fps_interval", "-1"))};

        SceneGraphConfig scene_graph_config{
            .max_distance_black = safe_stof(args.named_value("--max_distance_black", "200")),
            .small_aggregate_update_interval = safe_stoz(args.named_value("--small_aggregate_update_interval", "60")),
            .large_aggregate_update_interval = safe_stoz(args.named_value("--large_aggregate_update_interval", "3600"))};

        UiFocus ui_focus;
        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
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
        AEngine a_engine{scene_renderer, button_states.tap_buttons_};
        AContext context;
        ContextQueryGuard context_query_guard{context};
        ClearWrapperGuard clear_wrapper_guard;
        ARenderLoop render_loop{*app, a_engine};
        // AUi::RequestReadExternalStoragePermission();

        NotifyingJsonMacroArguments external_json_macro_arguments;
        // FifoLog fifo_log{10 * 1000};

        size_t args_num_renderings = safe_stoz(args.named_value("--num_renderings", "-1"));
        while (!render_loop.destroy_requested() && !unhandled_exceptions_occured()) {
            JsonMacroArgumentsObserverGuard smog{external_json_macro_arguments};
            num_renderings = args_num_renderings;
            ui_focus.submenu_numbers.clear();
            ui_focus.submenu_headers.clear();
            button_states.tap_buttons_.clear();

            PhysicsEngineConfig physics_engine_config{
                .dt = physics_dt * s,
                .control_fps = !args.has_named("--no_control_physics_fps"),
                .print_residual_time = args.has_named("--print_physics_residual_time"),
                // BVH
                .static_radius = safe_stof(args.named_value("--static_radius", "200")) * meters,
                .bvh_max_size = safe_stof(args.named_value("--bvh_max_size", "2")) * meters,
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
                .oversampling = safe_stoz(args.named_value("--oversampling", "2"))};

            SceneConfig scene_config{
                .render_config = render_config,
                .scene_graph_config = scene_graph_config,
                .physics_engine_config = physics_engine_config};

            SceneNodeResources scene_node_resources;
            ParticleResources particle_resources;
            SurfaceContactDb surface_contact_db;
            LayoutConstraints layout_constraints;
            {
                nlohmann::json j{
                    {"PRIMARY_SCENE_FLY", args.has_named("--fly")},
                    {"PRIMARY_SCENE_ROTATE", args.has_named("--rotate")},
                    {"PRIMARY_SCENE_PRINT_GAMEPAD_BUTTONS", args.has_named("--print_gamepad_buttons")},
                    {"PRIMARY_SCENE_DEPTH_FOG", !args.has_named("--no_depth_fog")},
                    {"PRIMARY_SCENE_LOW_PASS", args.has_named("--low_pass")},
                    {"PRIMARY_SCENE_HIGH_PASS", args.has_named("--high_pass")},
                    {"PRIMARY_SCENE_WITH_SKYBOX", true},
                    {"PRIMARY_SCENE_WITH_FLYING_LOGIC", true},
                    {"PRIMARY_SCENE_SAVE_PLAYBACK", args.has_named("--save_playback")},
                    {"FAR_PLANE", safe_stof(args.named_value("--far_plane", "10000"))},
                    {"IF_RECORD_TRACK", args.has_named("--record_track")},
                    {"IF_DEVEL", args.has_named("--devel_mode")},
                    {"IF_SHOW_DEBUG_WHEELS", args.has_named("--show_debug_wheels")},
                    {"IF_ANDROID", true},
                    {"SCENE_LIGHTMAP_WIDTH", safe_stoi(args.named_value("--scene_lightmap_width", "2048"))},
                    {"SCENE_LIGHTMAP_HEIGHT", safe_stoi(args.named_value("--scene_lightmap_height", "2048"))},
                    {"BLACK_LIGHTMAP_WIDTH", safe_stoi(args.named_value("--black_lightmap_width", "1024"))},
                    {"BLACK_LIGHTMAP_HEIGHT", safe_stoi(args.named_value("--black_lightmap_height", "1024"))}};
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
                RenderingContext primary_rendering_context{
                    .scene_node_resources = scene_node_resources,
                    .particle_resources = particle_resources,
                    .rendering_resources = rendering_resources,
                    .z_order = 0
                };
                RenderingContextGuard rcg{primary_rendering_context};

                RenderLogicGallery gallery;
                AssetReferences asset_references;
                RenderableScenes renderable_scenes;
                std::atomic_bool load_scene_finished = false;
                scene_renderer.set_scene(&renderable_scenes, &load_scene_finished);
                DestructionGuard dg{[&scene_renderer](){ scene_renderer.set_scene(nullptr, nullptr); }};

                FutureGuard loader_future_guard{loader_thread(
                    args,
                    gallery,
                    asset_references,
                    renderable_scenes,
                    search_path,
                    main_scene_filename,
                    next_scene_filename,
                    external_json_macro_arguments,
                    num_renderings,
                    render_set_fps,
                    surface_contact_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    ui_focus,
                    layout_constraints,
                    load_scene,
                    load_scene_finished)};
                render_loop.render_loop([&num_renderings](){return (num_renderings == 0) || unhandled_exceptions_occured();});
                if (args.has_named_value("--write_loaded_resources")) {
                    scene_node_resources.write_loaded_resources(args.named_value("--write_loaded_resources"));
                }
            }
            {
                std::scoped_lock lock{ui_focus.focuses.mutex};
                ui_focus.focuses.set_focuses({});
            }
            main_scene_filename = (std::string)next_scene_filename;
        }

        // if (!TimeGuard::is_empty(std::this_thread::get_id())) {
        //     std::cerr << "write svg" << std::endl;
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
