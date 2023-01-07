#include <Mlib/Arg_Parser.hpp>
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
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Pretty_Terminate.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Print_Gl_Version_Info.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/IRenderer.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Future_Guard.hpp>
#include <Mlib/Threads/Set_Thread_Name.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <filesystem>
#include <future>
#include <NDKHelper.h>

namespace fs = std::filesystem;

using namespace Mlib;

class SceneRenderer: public IRenderer {
public:
    SceneRenderer(
        AWindow& window,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const ParsedArgs &args)
    : window_{window},
      render_config_{render_config},
      scene_graph_config_{scene_graph_config},
      render_results_{render_results},
      args_{args}
    {}

    void load_resources() override {
        print_gl_version_info();
        // window_.set_frame_rate_if_supported(1.f / render_config_.min_dt);
    }
    void unload_resources() override {
        render_deallocator.deallocate();
    }
    void update_viewport() override {}
    void render(RenderEvent event, int width, int height) override {
        execute_render_gc();
        if (event != RenderEvent::LOOP) {
            return;
        }
        auto load_scene_finished = load_scene_finished_.lock();
        if (load_scene_finished == nullptr) {
            return;
        }
        auto renderable_scenes = renderable_scenes_.lock();
        if (renderable_scenes == nullptr) {
            return;
        }
        ViewportGuard vg{ width, height };
        if (*load_scene_finished) {
            (*renderable_scenes)["primary_scene"].render_logics_.render(
                width,
                height,
                render_config_,
                scene_graph_config_,
                render_results_,
                rrsd_.next(render_config_.motion_interpolation));
            if (args_.has_named("--single_threaded")) {
                for (auto& [_, r]: renderable_scenes->guarded_iterable()) {
                    if (!r.physics_set_fps_.paused()) {
                        r.physics_iteration_();
                    }
                }
            }
        } else if (renderable_scenes->contains("loading")) {
            auto &rs = (*renderable_scenes)["loading"];
            std::lock_guard lock{rs.scene_.delete_node_mutex()};
            if (rs.scene_.contains_node(rs.selected_cameras_.camera_node_name())) {
                rs.render_logics_.render(
                    width,
                    height,
                    render_config_,
                    scene_graph_config_,
                    render_results_,
                    rrsd_.next(render_config_.motion_interpolation));
            }
        } else {
            clear_color({0.2f, 0.2f, 0.2f, 1.f});
        }
    }

    void set_scene(
        std::weak_ptr<RenderableScenes> renderable_scenes,
        std::weak_ptr<std::atomic_bool> load_scene_finished)
    {
        renderable_scenes_ = std::move(renderable_scenes);
        load_scene_finished_ = std::move(load_scene_finished);
    }

private:
    const AWindow& window_;
    const RenderConfig& render_config_;
    const SceneGraphConfig& scene_graph_config_;
    RenderResults* render_results_;
    const ParsedArgs &args_;
    std::weak_ptr<RenderableScenes> renderable_scenes_;
    std::weak_ptr<std::atomic_bool> load_scene_finished_;
    RootRenderedSceneDescriptor rrsd_;
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
    RenderableScenes& renderable_scenes,
    const std::list<std::string>& search_path,
    const std::string& main_scene_filename,
    ThreadSafeString& next_scene_filename,
    SubstitutionMap& external_substitutions,
    std::atomic_size_t& num_renderings,
    SceneNodeResources& scene_node_resources,
    SurfaceContactDb& surface_contact_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    LoadScene& load_scene,
    RegexSubstitutionCache& rsc,
    const RenderingContext& primary_rendering_context,
    std::atomic_bool& load_scene_finished)
{
    return std::async(std::launch::async, [&](){
        try {
            set_thread_name("scene_loader");
            #ifndef WITHOUT_ALUT
            AudioResourceContext arc;
            #endif
            {
                RenderingContextGuard rrg{primary_rendering_context};
                #ifndef WITHOUT_ALUT
                AudioResourceContextGuard arcg{ arc };
                AudioListener::set_gain(safe_stof(args.named_value("--audio_gain", "1")));
                #endif
                // GlContextGuard gcg{ render2.window() };
                load_scene(
                    search_path,
                    main_scene_filename,
                    next_scene_filename,
                    external_substitutions,
                    num_renderings,
                    args.has_named("--verbose"),
                    rsc,
                    scene_node_resources,
                    surface_contact_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    ui_focus,
                    renderable_scenes);
                load_scene_finished = true;
                renderable_scenes["primary_scene"].instantiate_audio_listener();
            }

            print_debug_info(args, renderable_scenes);

            if (!args.has_named("--no_physics") &&
                !args.has_named("--single_threaded"))
            {
                for (auto& [n, r] : renderable_scenes.guarded_iterable()) {
                    r.delete_node_mutex_.clear_deleter_thread();
                    r.start_physics_loop(("Physics_" + n).substr(0, 15));
                }
            }
        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    });
}

void android_main(android_app* app) {
    set_log_level(LogLevel::ERROR);
    AUiGuard aui_guard{*app};
    // This throws exceptions internally, which is not supported
    // on Android.
    // register_pretty_terminate();
    // This currently has no effect, because the implementation is empty on Android.
    // Enabling floating point exceptions did not work on Android.
    enable_floating_point_exceptions();
    // This code sometimes caused crashes on some devices,
    // it is now moved to the NdkTestActivity Java class.
    // AUi::SetRequestedScreenOrientation(ScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE);

    const ArgParser parser(
        "Usage: render_scene_file working_directory scene.scn\n"
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
        "    [--double_buffer]\n"
        "    [--anisotropic_filtering_level <value>]\n"
        "    [--no_normalmaps]\n"
        "    [--no_physics ]\n"
        "    [--physics_dt <dt> ]\n"
        "    [--render_dt <dt> ]\n"
        "    [--no_control_physics_fps ]\n"
        "    [--no_control_render_fps ]\n"
        "    [--print_physics_residual_time]\n"
        "    [--print_render_residual_time]\n"
        "    [--draw_distance_add <value>]\n"
        "    [--far_plane <value>]\n"
        "    [--record_track]\n"
        "    [--devel_mode]\n"
        "    [--stiction_coefficient <x>]\n"
        "    [--friction_coefficient <x>]\n"
        "    [--lateral_stability <x>]\n"
        "    [--max_extra_friction <x>]\n"
        "    [--max_extra_w <x>]\n"
        "    [--longitudinal_friction_steepness <x>]\n"
        "    [--lateral_friction_steepness <x>]\n"
        "    [--no_slip <x>]\n"
        "    [--no_avoid_burnout]\n"
        "    [--wheel_penetration_depth <x>]\n"
        "    [--print_render_fps]\n"
        "    [--vfx]\n"
        "    [--no_depth_fog]\n"
        "    [--low_pass]\n"
        "    [--high_pass]\n"
        "    [--motion_interpolation]\n"
        "    [--no_render]\n"
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
        "    [--verbose]",
        {"--wire_frame",
         "--cull_faces",
         "--fly",
         "--rotate",
         "--no_physics",
         "--fullscreen",
         "--double_buffer",
         "--no_normalmaps",
         "--print_physics_residual_time",
         "--print_render_residual_time",
         "--print_render_fps",
         "--single_threaded",
         "--vfx",
         "--no_depth_fog",
         "--low_pass",
         "--high_pass",
         "--motion_interpolation",
         "--no_render",
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
         "--no_control_render_fps",
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
         "--stiction_coefficient",
         "--friction_coefficient",
         "--lateral_stability",
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
         "--write_loaded_resources"});
    try {
        const char* argv[] = {"appname", "/", "/levels/main/main.scn"};
        const auto args = parser.parsed(3, argv);

        args.assert_num_unamed(2);
        std::list<std::string> search_path = string_to_list(args.unnamed_value(0), Mlib::compile_regex(";"));
        std::string main_scene_filename = fs::absolute(args.unnamed_value(1)).string();

        #ifndef WITHOUT_ALUT
        AudioDevice audio_device;
        AudioContext audio_context{audio_device};
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
            .scene_lightmap_width = safe_stoi(args.named_value("--scene_lightmap_width", "2048")),
            .scene_lightmap_height = safe_stoi(args.named_value("--scene_lightmap_height", "2048")),
            .black_lightmap_width = safe_stoi(args.named_value("--black_lightmap_width", "512")),
            .black_lightmap_height = safe_stoi(args.named_value("--black_lightmap_height", "512")),
            .motion_interpolation = args.has_named("--motion_interpolation"),
            .fullscreen = args.has_named("--fullscreen"),
            .double_buffer = args.has_named("--double_buffer"),
            .anisotropic_filtering_level = safe_stou(args.named_value("--anisotropic_filtering_level", "0")),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = args.has_named("--show_mouse_cursor"),
            .swap_interval = safe_stoi(args.named_value("--swap_interval", "1")),
            .print_fps = args.has_named("--print_render_fps"),
            .control_fps = !args.has_named("--no_control_render_fps"),
            .print_residual_time = args.has_named("--print_render_residual_time"),
            .min_dt = safe_stof(args.named_value("--render_dt", "0.01667")),
            .draw_distance_add = safe_stof(args.named_value("--draw_distance_add", "INFINITY"))};

        SceneGraphConfig scene_graph_config{
            .max_distance_black = safe_stof(args.named_value("--max_distance_black", "200")),
            .small_aggregate_update_interval = safe_stoz(args.named_value("--small_aggregate_update_interval", "60")),
            .large_aggregate_update_interval = safe_stoz(args.named_value("--large_aggregate_update_interval", "3600"))};

        AWindow window{*app};
        // Declared as first class to let destructors of other classes succeed.
        SceneRenderer scene_renderer{
            window,
            render_config,
            scene_graph_config,
            nullptr,    // render_results
            args};
        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        AEngine a_engine{scene_renderer, button_states.tap_buttons_};
        ClearWrapperGuard clear_wrapper_guard;
        ARenderLoop render_loop{*app, a_engine};
        AContext context;
        ContextQueryGuard context_query_guard{context};
        // AUi::RequestReadExternalStoragePermission();

        UiFocus ui_focus;
        SubstitutionMap external_substitutions;
        // FifoLog fifo_log{10 * 1000};

        size_t args_num_renderings = safe_stoz(args.named_value("--num_renderings", "-1"));
        while (!render_loop.destroy_requested() && !unhandled_exceptions_occured()) {
            num_renderings = args_num_renderings;
            ui_focus.submenu_numbers.clear();
            ui_focus.submenu_headers.clear();
            button_states.tap_buttons_.clear();

            PhysicsEngineConfig physics_engine_config{
                .dt = safe_stof(args.named_value("--physics_dt", "0.01667")) * s,
                .control_fps = !args.has_named("--no_control_physics_fps"),
                .print_residual_time = args.has_named("--print_physics_residual_time"),
                .stiction_coefficient = safe_stof(args.named_value("--stiction_coefficient", "0.5")),
                .friction_coefficient = safe_stof(args.named_value("--friction_coefficient", "0.5")),
                .avoid_burnout = !args.has_named("--no_avoid_burnout"),
                .no_slip = args.has_named("--no_slip"),
                .lateral_stability = safe_stof(args.named_value("--lateral_stability", "1")),
                .max_extra_friction = safe_stof(args.named_value("--max_extra_friction", "0")),
                .max_extra_w = safe_stof(args.named_value("--max_extra_w", "0")),
                .longitudinal_friction_steepness = safe_stof(args.named_value("--longitudinal_friction_steepness", "5")),
                .lateral_friction_steepness = safe_stof(args.named_value("--lateral_friction_steepness", "7")),
                .wheel_penetration_depth = safe_stof(args.named_value("--wheel_penetration_depth", "0.25")),
                .static_radius = safe_stof(args.named_value("--static_radius", "200")),
                .bvh_max_size = safe_stof(args.named_value("--bvh_max_size", "50")),
                .oversampling = safe_stoz(args.named_value("--oversampling", "2"))};

            SceneConfig scene_config{
                .render_config = render_config,
                .scene_graph_config = scene_graph_config,
                .physics_engine_config = physics_engine_config};

            SceneNodeResources scene_node_resources;
            SurfaceContactDb surface_contact_db;
            {
                std::map<std::string, std::string> sstr{
                    {"PRIMARY_SCENE_FLY", std::to_string(args.has_named("--fly"))},
                    {"PRIMARY_SCENE_ROTATE", std::to_string(args.has_named("--rotate"))},
                    {"PRIMARY_SCENE_PRINT_GAMEPAD_BUTTONS", std::to_string(args.has_named("--print_gamepad_buttons"))},
                    {"PRIMARY_SCENE_DEPTH_FOG", std::to_string(!args.has_named("--no_depth_fog"))},
                    {"PRIMARY_SCENE_LOW_PASS", std::to_string(args.has_named("--low_pass"))},
                    {"PRIMARY_SCENE_HIGH_PASS", std::to_string(args.has_named("--high_pass"))},
                    {"PRIMARY_SCENE_WITH_SKYBOX", "1"},
                    {"PRIMARY_SCENE_WITH_FLYING_LOGIC", "1"},
                    {"PRIMARY_SCENE_CLEAR_MODE", "color_and_depth"},
                    {"FAR_PLANE", std::to_string(safe_stof(args.named_value("--far_plane", "10000")))},
                    {"IF_RECORD_TRACK", args.has_named("--record_track") ? "" : "#"},
                    {"IF_DEVEL", args.has_named("--devel_mode") ? "" : "#"},
                    {"IF_RELEASE", args.has_named("--devel_mode") ? "#" : ""},
                    {"IF_SHOW_DEBUG_WHEELS", args.has_named("--show_debug_wheels") ? "" : "#"},
                    {"IF_ANDROID", ""}
                };
                external_substitutions.merge(SubstitutionMap{std::move(sstr)});
            }
            // Must be above "load_scene" in case user functions want to
            // call macros in their destructors.
            RegexSubstitutionCache rsc;
            // "load_scene" must be above "renderable_scenes", because the "RenderableScene" background
            // threads have lambda functions operating on the "load_scene.macro_recorder_" object.
            // In case of an exception in the main thread, destruction of "load_scene" must therefore happen
            // after the destruction of "renderable_scenes".
            LoadScene load_scene;
            ThreadSafeString next_scene_filename;
            {
                auto renderable_scenes = std::make_shared<RenderableScenes>();
                RenderingContext primary_rendering_context{
                    .scene_node_resources = scene_node_resources,
                    .rendering_resources = std::make_shared<RenderingResources>(
                        "primary_rendering_resources",
                        render_config.anisotropic_filtering_level),
                    .z_order = 0
                };

                auto load_scene_finished = std::make_shared<std::atomic_bool>(false);
                scene_renderer.set_scene(renderable_scenes, load_scene_finished);
                std::future<void> render_future;

                FutureGuard loader_future_guard{loader_thread(
                    args,
                    *renderable_scenes,
                    search_path,
                    main_scene_filename,
                    next_scene_filename,
                    external_substitutions,
                    num_renderings,
                    scene_node_resources,
                    surface_contact_db,
                    scene_config,
                    button_states,
                    cursor_states,
                    scroll_wheel_states,
                    ui_focus,
                    load_scene,
                    rsc,
                    primary_rendering_context,
                    *load_scene_finished)};
                {
                    RenderingContextGuard rrg{primary_rendering_context};
                    render_loop.render_loop([&num_renderings](){return (num_renderings == 0) || unhandled_exceptions_occured();});
                }
                if (args.has_named_value("--write_loaded_resources")) {
                    scene_node_resources.write_loaded_resources(args.named_value("--write_loaded_resources"));
                }
            }
            {
                std::unique_lock lock{ui_focus.focuses.mutex};
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
        std::string remaining_msg = e.what();
        while (!remaining_msg.empty()) {
            auto current_msg = remaining_msg.substr(0, 1000);
            LOGE("Runtime error: %s", current_msg.c_str());
            AUi::ShowMessage("Error", current_msg);
            remaining_msg = remaining_msg.substr(current_msg.size());
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::abort();
    }
    if (unhandled_exceptions_occured()) {
        std::stringstream sstr;
        print_unhandled_exceptions(sstr);
        LOGE("Unhandled exception(s): %s", sstr.str().c_str());
        AUi::ShowMessage("Error", sstr.str());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::abort();
    }
}
