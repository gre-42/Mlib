#include <Mlib/Arg_Parser.hpp>
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#endif
#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Renderer.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Future_Guard.hpp>
#include <Mlib/Threads/Set_Thread_Name.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <filesystem>
#include <future>

namespace fs = std::filesystem;

using namespace Mlib;

std::future<void> render_thread(
    const ParsedArgs& args,
    RenderableScenes& renderable_scenes,
    std::atomic_bool& load_scene_finished,
    const RenderingContext& primary_rendering_context,
    Renderer& renderer,
    const SceneConfig& scene_config)
{
    return std::async(std::launch::async, [&](){
        try {
            set_thread_name("rendr_and_evnts");
            LambdaRenderLogic lrl{
                [&](const LayoutConstraintParameters& lx,
                    const LayoutConstraintParameters& ly,
                    const RenderConfig& render_config,
                    const SceneGraphConfig& scene_graph_config,
                    RenderResults* render_results,
                    const RenderedSceneDescriptor& frame_id)
                {
                    if (load_scene_finished) {
                        renderable_scenes["primary_scene"].render_logics_.render(
                            lx,
                            ly,
                            render_config,
                            scene_graph_config,
                            render_results,
                            frame_id);
                        if (args.has_named("--single_threaded")) {
                            for (auto& [_, r] : renderable_scenes.guarded_iterable()) {
                                if (!r.physics_set_fps_.paused()) {
                                    r.physics_iteration_();
                                }
                            }
                        }
                    } else if (renderable_scenes.contains("loading")) {
                        auto& rs = renderable_scenes["loading"];
                        std::lock_guard lock{rs.scene_.delete_node_mutex()};
                        if (rs.scene_.contains_node(rs.selected_cameras_.camera_node_name())) {
                            rs.render_logics_.render(
                                lx,
                                ly,
                                render_config,
                                scene_graph_config,
                                render_results,
                                frame_id);
                        }
                    }
                }};
            RenderingContextGuard rrg{primary_rendering_context};
            renderer.render(lrl, scene_config.scene_graph_config);
        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    });
}

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
    SubstitutionMap& external_substitutions,
    std::atomic_size_t& num_renderings,
    SceneNodeResources& scene_node_resources,
    SurfaceContactDb& surface_contact_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    LayoutConstraints& layout_constraints,
    LoadScene& load_scene,
    RegexSubstitutionCache& rsc,
    const RenderingContext& primary_rendering_context,
    std::atomic_bool& load_scene_finished,
    const Render2& render2)
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
                    layout_constraints,
                    render2.glfw_window(),
                    gallery,
                    asset_references,
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

void main_func(
    const ParsedArgs& args,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    size_t args_num_renderings,
    Renderer* renderer)
{
    if (args.has_named("--no_render")) {
        std::cout << "Exiting because of --no_render" << std::endl;
    } else {
        EventHandler(*renderer, &button_states, &cursor_states, &scroll_wheel_states);
        if (args_num_renderings != SIZE_MAX) {
            std::cout << "Exiting because of --num_renderings" << std::endl;
        }
        // if (!render2.window_should_close() && !unhandled_exceptions_occured()) {
        //     ui_focus.focuses = {Focus::SCENE, Focus::LOADING};
        //     num_renderings = 1;
        //     render2(
        //         rs.render_logics_,
        //         scene_config.scene_graph_config);
        //     ui_focus.focuses = {};
        // }
    }
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: render_scene_file working_directory scene.scn\n"
        "    [--app_reldir]\n"
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
        "    [--no_vfx]\n"
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
         "--no_vfx",
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
        {"--app_reldir",
         "--swap_interval",
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
        const auto args = parser.parsed(argc, argv);
        if (args.has_named_value("--app_reldir")) {
            set_app_reldir(args.named_value("--app_reldir"));
            create_directories(get_appdata_directory());
        }

        args.assert_num_unnamed(2);
        std::list<std::string> search_path = string_to_list(args.unnamed_value(0), Mlib::compile_regex(";"));
        std::string main_scene_filename = fs::absolute(args.unnamed_value(1)).string();

#ifndef WITHOUT_ALUT
        AudioDevice audio_device;
        AudioContext audio_context{audio_device};
#endif

        std::atomic_size_t num_renderings;
        RenderConfig render_config{
            .nsamples_msaa = safe_stoi(args.named_value("--nsamples_msaa", "2")),
            .lightmap_nsamples_msaa = safe_stoi(args.named_value("--lightmap_nsamples_msaa", "4")),
            .min_sample_shading = safe_stof(args.named_value("--min_sample_shading", "0")),
            .vfx = !args.has_named("--no_vfx"),
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
            .anisotropic_filtering_level = safe_stou(args.named_value("--anisotropic_filtering_level", "8")),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = args.has_named("--show_mouse_cursor"),
            .swap_interval = safe_stoi(args.named_value("--swap_interval", "1")),
            .print_fps = args.has_named("--print_render_fps"),
            .control_fps = !args.has_named("--no_control_render_fps"),
            .print_residual_time = args.has_named("--print_render_residual_time"),
            .min_dt = safe_stof(args.named_value("--render_dt", "0.01667")),
            .draw_distance_add = safe_stof(args.named_value("--draw_distance_add", "INFINITY"))};
        // Declared as first class to let destructors of other classes succeed.
        Render2 render2{
            render_config,
            num_renderings,
            nullptr};
        render2.print_hardware_info();

        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        UiFocus ui_focus;
        SubstitutionMap external_substitutions;
        // FifoLog fifo_log{10 * 1000};

        size_t args_num_renderings = safe_stoz(args.named_value("--num_renderings", "-1"));
        while (!render2.window_should_close() && !unhandled_exceptions_occured()) {
            num_renderings = args_num_renderings;
            ui_focus.submenu_numbers.clear();
            ui_focus.submenu_headers.clear();

            SceneGraphConfig scene_graph_config{
                .max_distance_black = safe_stof(args.named_value("--max_distance_black", "200")),
                .small_aggregate_update_interval = safe_stoz(args.named_value("--small_aggregate_update_interval", "60")),
                .large_aggregate_update_interval = safe_stoz(args.named_value("--large_aggregate_update_interval", "3600"))};

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
            LayoutConstraints layout_constraints;
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
                    {"IF_ANDROID", "#"}
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
                RenderLogicGallery gallery;
                AssetReferences asset_references;
                RenderableScenes renderable_scenes;
                RenderingContext primary_rendering_context{
                    .scene_node_resources = scene_node_resources,
                    .rendering_resources = std::make_shared<RenderingResources>(
                        "primary_rendering_resources",
                        render_config.anisotropic_filtering_level),
                    .z_order = 0
                };

                std::atomic_bool load_scene_finished = false;
                std::future<void> render_future;
                std::unique_ptr<Renderer> renderer;
                if (!args.has_named("--no_render")) {
                    renderer = std::make_unique<Renderer>(render2.generate_renderer());
                    render_future = render_thread(
                        args,
                        renderable_scenes,
                        load_scene_finished,
                        primary_rendering_context,
                        *renderer,
                        scene_config);
                }
                FutureGuard render_future_guard{std::move(render_future)};
                FutureGuard loader_future_guard{loader_thread(
                    args,
                    gallery,
                    asset_references,
                    renderable_scenes,
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
                    layout_constraints,
                    load_scene,
                    rsc,
                    primary_rendering_context,
                    load_scene_finished,
                    render2)};
                try {
                    main_func(
                        args,
                        button_states,
                        cursor_states,
                        scroll_wheel_states,
                        args_num_renderings,
                        renderer.get());
                } catch (...) {
                    add_unhandled_exception(std::current_exception());
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
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        return 1;
    }
    return 0;
}
