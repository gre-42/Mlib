#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Verbose_Vector.hpp>
#include <Mlib/Audio/Audio_Context.hpp>
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/Audio_Distance_Model.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Audio/List_Audio_Devices.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translators.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Light_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/IWindow.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Descriptions.hpp>
#include <Mlib/Render/Key_Bindings/Make_Key_Binding.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Menu_Logic.hpp>
#include <Mlib/Render/Render_Logics/Window_Logic.hpp>
#include <Mlib/Render/Renderer.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Ui/Static_Renderable_Hider.hpp>
#include <Mlib/Render/Ui/Tty_Renderable_Hider.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Physics_Scenes.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

static const auto& g = make_gamepad_button;

std::unique_ptr<JThread> render_thread(
    const ParsedArgs& args,
    ButtonStates& button_states,
    LockableKeyConfigurations& key_configurations,
    PhysicsScenes& physics_scenes,
    RenderableScenes& renderable_scenes,
    std::atomic_bool& load_scene_finished,
    Renderer& renderer,
    const SceneConfig& scene_config,
    MenuLogic& menu_logic)
{
    return std::make_unique<JThread>([&](){
        try {
            ThreadInitializer ti{ "Render", ThreadAffinity::POOL };
            bool last_load_scene_finished = false;
            LambdaRenderLogic lrl{
                [&](const LayoutConstraintParameters& lx,
                    const LayoutConstraintParameters& ly,
                    const RenderConfig& render_config,
                    const SceneGraphConfig& scene_graph_config,
                    RenderResults* render_results,
                    const RenderedSceneDescriptor& frame_id)
                {
                    if (scene_graph_config.renderable_hider != nullptr) {
                        scene_graph_config.renderable_hider->process_input();
                    }
                    menu_logic.handle_events();
                    if (load_scene_finished) {
                        execute_render_allocators();
                        auto& rs = physics_scenes["primary_scene"];
                        rs.scene_.wait_for_cleanup();
                        if (!last_load_scene_finished && 
                            !args.has_named("--no_physics") &&
                            !args.has_named("--single_threaded"))
                        {
                            for (auto& [n, r] : physics_scenes.guarded_iterable()) {
                                r.delete_node_mutex_.clear_deleter_thread();
                                r.start_physics_loop(("Phys_" + n).substr(0, 15), ThreadAffinity::POOL);
                            }
                            last_load_scene_finished = true;
                        }
                        renderable_scenes.render_toplevel(
                            lx,
                            ly,
                            render_config,
                            scene_graph_config,
                            render_results,
                            frame_id);
                        if (args.has_named("--single_threaded")) {
                            for (auto& [_, r] : physics_scenes.guarded_iterable()) {
                                SetDeleterThreadGuard set_deleter_thread_guard{ r.scene_.delete_node_mutex() };
                                if (!r.physics_set_fps_.paused()) {
                                    r.physics_iteration(std::chrono::steady_clock::now());
                                }
                                r.physics_set_fps_.execute_oldest_funcs();
                            }
                        }
                    } else if (auto rs = renderable_scenes.try_get("loading"); rs != nullptr) {
                        execute_render_allocators();
                        rs->physics_scene_->scene_.wait_for_cleanup();
                        if (rs->selected_cameras_.camera_node_exists()) {
                            rs->render_toplevel(
                                lx,
                                ly,
                                render_config,
                                scene_graph_config,
                                render_results,
                                frame_id);
                        }
                    } else {
                        clear_color({0.2f, 0.2f, 0.2f, 1.f});
                    }
                }};
            ReadPixelsLogic rpl{ lrl, button_states, key_configurations, ReadPixelsRole::SCREENSHOT };
            ClearWrapperGuard clear_wrapper_guard;
            renderer.render(rpl, scene_config.scene_graph_config);
        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    });
}

void print_debug_info(
    const ParsedArgs& args,
    PhysicsScenes& physics_scenes)
{
    if (args.has_named("--print_search_time") ||
        args.has_named("--print_compression_ratio") ||
        args.has_named("--optimize_search_time") ||
        args.has_named("--plot_triangle_bvh"))
    {
        for (const auto& [n, r] : physics_scenes.guarded_iterable()) {
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
    PhysicsScenes& physics_scenes,
    RenderableScenes& renderable_scenes,
    LoadScene& load_scene,
    std::atomic_bool& load_scene_finished,
    std::chrono::steady_clock::duration render_delay,
    std::chrono::steady_clock::duration velocity_dt)
{
    return JThread{[&, render_delay, velocity_dt](){
        try {
            ThreadInitializer ti{"Scene loader", ThreadAffinity::POOL};
            AudioResourceContext arc;
            {
                AudioResourceContextGuard arcg{ arc };
                AudioListener::set_gain(safe_stof(args.named_value("--audio_gain", "1")));
                // GlContextGuard gcg{ render2.window() };
                load_scene();
                renderable_scenes["primary_scene_0"].instantiate_audio_listener(
                    render_delay,
                    velocity_dt);
                if (!args.has_named("--no_physics")) {
                    if (args.has_named("--no_render")) {
                        for (auto& [n, r] : physics_scenes.guarded_iterable()) {
                            r.delete_node_mutex_.clear_deleter_thread();
                            r.start_physics_loop(("Phys_" + n).substr(0, 15), ThreadAffinity::POOL);
                        }
                    } else if (args.has_named("--single_threaded")) {
                        for (auto& [n, r] : physics_scenes.guarded_iterable()) {
                            r.scene_.delete_node_mutex().clear_deleter_thread();
                        }
                    }
                }
                load_scene_finished = true;
            }

            print_debug_info(args, physics_scenes);

        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    }};
}

static void main_func(
    std::function<void(uint32_t)> char_callback,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    size_t args_num_renderings,
    Renderer* renderer,
    const InputConfig& input_config,
    const std::function<void()>& event_callback)
{
    if (renderer == nullptr) {
        linfo() << "Exiting because of --no_render";
    } else {
        handle_events(*renderer, &char_callback, &button_states, &cursor_states, &scroll_wheel_states, input_config, event_callback);
        if (args_num_renderings != SIZE_MAX) {
            linfo() << "Exiting because of --num_renderings";
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
    reserve_realtime_threads(0);
    ThreadInitializer ti{"Main", ThreadAffinity::POOL};

    const char* help =
        "Usage: render_scene_file working_directory scene.scn.json\n"
        "    [--help]\n"
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
        "    [--scene_water_waves_width <width>]\n"
        "    [--scene_water_waves_height <height>]\n"
        "    [--scene_sea_spray_width <width>]\n"
        "    [--scene_sea_spray_height <height>]\n"
        "    [--fullscreen]\n"
        "    [--no_double_buffer]\n"
        "    [--anisotropic_filtering_level <value>]\n"
        "    [--no_normalmaps]\n"
        "    [--no_physics]\n"
        "    [--physics_dt <dt>]\n"
        "    [--render_dt <dt>]\n"
        "    [--input_polling_interval <dt>]\n"
        "    [--render_max_residual_time <dt>]\n"
        "    [--no_control_physics_fps ]\n"
        "    [--print_render_fps_interval <n>]\n"
        "    [--fullscreen_refresh_rate <Hz>]\n"
        "    [--print_physics_residual_time]\n"
        "    [--print_render_residual_time]\n"
        "    [--draw_distance_add <value>]\n"
        "    [--far_plane <value>]\n"
        "    [--record_track_basename <value>]\n"
        "    [--flavor <flavor>]\n"
        "    [--devel_mode]\n"
        "    [--enable_ridge_map]\n"
        "    [--hand_brake_velocity <x>]\n"
        "    [--stiction_coefficient <x>]\n"
        "    [--friction_coefficient <x>]\n"
        "    [--max_extra_friction <x>]\n"
        "    [--max_extra_w <x>]\n"
        "    [--longitudinal_friction_steepness <x>]\n"
        "    [--lateral_friction_steepness <x>]\n"
        "    [--no_slip <x>]\n"
        "    [--no_avoid_burnout]\n"
        "    [--wheel_penetration_depth <x>]\n"
        "    [--sparse_triangle_cluster_width <width>]\n"
        "    [--medium_triangle_cluster_width <width>]\n"
        "    [--dense_triangle_cluster_width <width>]\n"
        "    [--object_cluster_width <width>]\n"
        "    [--no_vfx]\n"
        "    [--depth_fog_vfx]\n"
        "    [--low_pass]\n"
        "    [--high_pass]\n"
        "    [--bloom_x <niterations>]\n"
        "    [--bloom_y <niterations>]\n"
        "    [--bloom_threshold <threshold>]\n"
        "    [--bloom_std <stddev>]\n"
        "    [--bloom_intensities <intensities>]\n"
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
        "    [--show_global_log]\n"
        "    [--write_loaded_resources <dir>]\n"
        "    [--audio_frequency <value>]\n"
        "    [--audio_alpha <value>]\n"
        "    [--audio_distance_model <value>]\n"
        "    [--user_count <n>]\n"
        "    [--remote_site_id <id>]\n"
        "    [--remote_role {server,client}]\n"
        "    [--remote_ip <ip>]\n"
        "    [--remote_port <port>]\n"
        "    [--tty_hider]\n"
        "    [--show_only <name>]\n"
        "    [--check_gl_errors]\n"
        "    [--verbose]";
    const ArgParser parser(
        help,
        {"--help",
         "--wire_frame",
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
         "--no_vfx",
         "--depth_fog_vfx",
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
         "--fxaa",
         "--tty_hider",
         "--check_gl_errors",
         "--verbose"},
        {"--app_reldir",
         "--record_track_basename",
         "--flavor",
         "--swap_interval",
         "--fullscreen_refresh_rate",
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
         "--scene_water_waves_width",
         "--scene_water_waves_height",
         "--scene_sea_spray_width",
         "--scene_sea_spray_height",
         "--static_radius",
         "--bvh_max_size",
         "--physics_dt",
         "--nsubsteps",
         "--render_dt",
         "--input_polling_interval",
         "--render_max_residual_time",
         "--hand_brake_velocity",
         "--stiction_coefficient",
         "--friction_coefficient",
         "--max_extra_w",
         "--max_extra_friction",
         "--longitudinal_friction_steepness",
         "--lateral_friction_steepness",
         "--wheel_penetration_depth",
         "--sparse_triangle_cluster_width",
         "--medium_triangle_cluster_width",
         "--dense_triangle_cluster_width",
         "--object_cluster_width",
         "--draw_distance_add",
         "--far_plane",
         "--num_renderings",
         "--audio_gain",
         "--show_debug_wheels",
         "--show_global_log",
         "--write_loaded_resources",
         "--audio_frequency",
         "--audio_alpha",
         "--audio_distance_model",
         "--user_count",
         "--remote_site_id",
         "--remote_role",
         "--remote_ip",
         "--remote_port",
         "--bloom_x",
         "--bloom_y",
         "--bloom_threshold",
         "--bloom_std",
         "--bloom_intensities",
         "--show_only"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        if (args.has_named_value("--app_reldir")) {
            set_app_reldir(args.named_value("--app_reldir"));
            create_directories(get_appdata_directory());
        }

        args.assert_num_unnamed(2);
        auto search_path = string_to_list(args.unnamed_value(0), Mlib::compile_regex(";"));
        auto initial_main_scene_filename = fs::absolute(args.unnamed_value(1)).string();
        auto main_scene_filename = initial_main_scene_filename;

        if (args.has_named("--check_gl_errors")) {
            check_gl_errors(CheckErrors::ENABLED);
        }
        list_audio_devices(linfo(LogFlags::NO_APPEND_NEWLINE).ref());
        AudioDevice audio_device;
        linfo() << "Selected audio device: " << audio_device.get_name();
        AudioContext audio_context{audio_device, safe_stou(args.named_value("--audio_frequency", "0"))};
        linfo() << "Audio frequency: " << audio_device.get_frequency();
        AudioScene::set_default_alpha(safe_stof(args.named_value("--audio_alpha", "0.1")));
        AudioScene::set_distance_model(audio_distance_model_from_string(args.named_value("--audio_distance_model", "inverse_distance_clamped")));

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
            .windowed_width = safe_stoi(args.named_value("--windowed_width", "800")),
            .windowed_height = safe_stoi(args.named_value("--windowed_height", "600")),
            .fullscreen_width = safe_stoi(args.named_value("--fullscreen_width", "0")),
            .fullscreen_height = safe_stoi(args.named_value("--fullscreen_height", "0")),
            .motion_interpolation = args.has_named("--motion_interpolation"),
            .fullscreen = args.has_named("--fullscreen"),
            .double_buffer = !args.has_named("--no_double_buffer"),
            .anisotropic_filtering_level = safe_stou(args.named_value("--anisotropic_filtering_level", "8")),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = args.has_named("--show_mouse_cursor"),
            .swap_interval = safe_stoi(args.named_value("--swap_interval", "1")),
            .fullscreen_refresh_rate = safe_stoi(args.named_value("--fullscreen_refresh_rate", "0")),
            .draw_distance_add = safe_stof(args.named_value("--draw_distance_add", "inf"))};
        InputConfig input_config{
            .polling_interval_seconds = safe_stof(args.named_value("--input_polling_interval", "0.00416667"))
        };
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
        // Declared as first class to let destructors of other classes succeed.
        Render render{
            render_config,
            input_config,
            num_renderings,
            render_set_fps.set_fps,
            [&render_set_fps]() { return render_set_fps.ft.frame_time(); } };
        render.print_hardware_info(linfo(LogFlags::NO_APPEND_NEWLINE).ref());

        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        LockableKeyConfigurations confirm_key_configurations;
        uint32_t ngamepads = 2;
        VerboseVector<ButtonPress> confirm_button_press("Confirm button press");
        confirm_button_press.reserve(ngamepads);
        {
            auto locked_key_configs = confirm_key_configurations
                .lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
            BaseKeyCombination confirm_key_combination_0{{{
                BaseKeyBinding{
                    .key = "ENTER",
                    .gamepad_button = g(0, "A"),
                    .tap_button = g(0, "START")}}}};
            locked_key_configs->insert(0, "confirm", { std::move(confirm_key_combination_0) });
            confirm_button_press.emplace_back(button_states, confirm_key_configurations, 0, "confirm", "");
            for (uint32_t i = 1; i < ngamepads; ++i) {
                BaseKeyCombination confirm_key_combination_i{{{
                    BaseKeyBinding{.gamepad_button = g(i, "A")}}}};
                locked_key_configs->insert(i, "confirm", { std::move(confirm_key_combination_i) });
                confirm_button_press.emplace_back(button_states, confirm_key_configurations, i, "confirm", "");
            }
        }
        UiFocuses ui_focuses{ get_path_in_appdata_directory({"focus.json"}) };
        ui_focuses.try_load();
        NotifyingJsonMacroArguments external_json_macro_arguments;
        // FifoLog fifo_log{10 * 1000};

        WindowUserClass window_user_object{
            .window_position{
                .fullscreen_width = render_config.fullscreen_width,
                .fullscreen_height = render_config.fullscreen_height,
            },
            .button_states = button_states,
            .exit_on_escape = false};
        MenuUserClass menu_user_object{
            .button_states = button_states,
            .ui_focuses = ui_focuses};
        WindowLogic window_logic{
            render.glfw_window(),
            window_user_object};
        MenuLogic menu_logic{menu_user_object};

        {
            auto record_track_basename = args.try_named_value("--record_track_basename");
            nlohmann::json j{
                {"primary_scene_fly", args.has_named("--fly")},
                {"primary_scene_rotate", args.has_named("--rotate")},
                {"primary_scene_depth_fog", args.has_named("--depth_fog_vfx")},
                {"primary_scene_low_pass", args.has_named("--low_pass")},
                {"primary_scene_high_pass", args.has_named("--high_pass")},
                {"primary_scene_bloom_iterations", FixedArray<unsigned int, 2>{
                    safe_stou(args.named_value("--bloom_x", "3")),
                    safe_stou(args.named_value("--bloom_y", "3"))}},
                {"primary_scene_bloom_thresholds", fixed_full<float, 3>(
                    safe_stof(args.named_value("--bloom_threshold", "1")))},
                {"primary_scene_bloom_std", fixed_full<float, 2>(
                    safe_stof(args.named_value("--bloom_std", "4")))},
                {"primary_scene_bloom_intensities", fixed_full<float, 3>(
                    safe_stof(args.named_value("--bloom_intensities", "4")))},
                {"primary_scene_bloom_mode", args.named_value("--bloom_mode", "sky")},
                {"primary_scene_with_skybox", true},
                {"primary_scene_with_flying_logic", true},
                {"primary_scene_save_playback", args.has_named("--save_playback")},
                {"far_plane", safe_stof(args.named_value("--far_plane", "10000"))},
                {"record_track_basename", (record_track_basename == nullptr)
                    ? nlohmann::json()
                    : nlohmann::json(*record_track_basename)},
                {"if_devel", args.has_named("--devel_mode")},
                {"if_show_debug_wheels", args.has_named("--show_debug_wheels")},
                {"if_show_global_log", args.has_named("--show_global_log")},
                {"if_android", false},
                {"flavor", args.named_value("--flavor", "main")},
                {"scene_lightmap_width", safe_stoi(args.named_value("--scene_lightmap_width", "2048"))},
                {"scene_lightmap_height", safe_stoi(args.named_value("--scene_lightmap_height", "2048"))},
                {"black_lightmap_width", safe_stoi(args.named_value("--black_lightmap_width", "1024"))},
                {"black_lightmap_height", safe_stoi(args.named_value("--black_lightmap_height", "1024"))},
                {"scene_skidmarks_width", safe_stoi(args.named_value("--scene_skidmarks_width", "2048"))},
                {"scene_skidmarks_height", safe_stoi(args.named_value("--scene_skidmarks_height", "2048"))},
                {"scene_water_waves_width", safe_stoi(args.named_value("--scene_water_waves_width", "512"))},
                {"scene_water_waves_height", safe_stoi(args.named_value("--scene_water_waves_height", "512"))},
                {"scene_sea_spray_width", safe_stoi(args.named_value("--scene_sea_spray_width", "2048"))},
                {"scene_sea_spray_height", safe_stoi(args.named_value("--scene_sea_spray_height", "2048"))},
                {"selected_user_count", safe_sto<uint32_t>(args.named_value("--user_count", "1"))},
                {"remote_role", args.named_value("--remote_role", "none")},
                {"sparse_triangle_cluster_width", safe_stof(args.named_value("--sparse_triangle_cluster_width", "3e3"))},
                {"medium_triangle_cluster_width", safe_stof(args.named_value("--medium_triangle_cluster_width", "700"))},
                {"dense_triangle_cluster_width", safe_stof(args.named_value("--dense_triangle_cluster_width", "250"))},
                {"object_cluster_width", safe_stof(args.named_value("--object_cluster_width", "500"))}};
                if (args.has_named_value("--remote_role")) {
                    j["remote_params"] = RemoteParams{
                        safe_stox<RemoteSiteId>(args.named_value("--remote_site_id")),
                        remote_role_from_string(args.named_value("--remote_role")),
                        args.named_value("--remote_ip"),
                        safe_stox<uint16_t>(args.named_value("--remote_port"))};
                } else {
                    j["remote_params"] = nlohmann::json();
                }
            external_json_macro_arguments.merge_and_notify(JsonMacroArguments{std::move(j)});
        }
        size_t args_num_renderings = safe_stoz(args.named_value("--num_renderings", "-1"));
        while (!render.window_should_close() && !unhandled_exceptions_occured()) {
            num_renderings = args_num_renderings;
            ui_focuses.clear();

            TtyRenderableHider tty_renderable_hider{ button_states };
            StaticRenderableHider static_renderable_hider{ args.named_value("--show_only", "") };
            IRenderableHider* renderable_hider = nullptr;
            if (args.has_named("--tty_hider")) {
                renderable_hider = &tty_renderable_hider;
            }
            if (args.has_named_value("--show_only")) {
                if (renderable_hider != nullptr) {
                    THROW_OR_ABORT("Both --tty_hider and --show_only were specified");
                }
                renderable_hider = &static_renderable_hider;
            }

            SceneGraphConfig scene_graph_config{
                .max_distance_black = safe_stof(args.named_value("--max_distance_black", "200")) * meters,
                .small_aggregate_update_interval = safe_stoz(args.named_value("--small_aggregate_update_interval", "60")),
                .large_max_offset_deviation = safe_stof(args.named_value("--large_max_offset_deviation", "200")) * meters,
                .renderable_hider = renderable_hider };

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
                .hand_brake_velocity = safe_stof(args.named_value("--hand_brake_velocity", "5")) * kph,
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
            
            // "load_scene" must be above "renderable_scenes", because the "RenderableScene" background
            // threads have lambda functions operating on the "load_scene.macro_recorder_" object.
            // In case of an exception in the main thread, destruction of "load_scene" must therefore happen
            // after the destruction of "renderable_scenes".
            std::unique_ptr<LoadScene> load_scene;
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

                Users users;
                RenderLogicGallery gallery;
                AssetReferences asset_references;
                Translators translators{ asset_references, external_json_macro_arguments };
                RenderableScenes renderable_scenes;
                PhysicsScenes physics_scenes;

                std::atomic_bool load_scene_finished = false;
                std::unique_ptr<Renderer> renderer;
                std::unique_ptr<JThread> render_future;
                if (!args.has_named("--no_render")) {
                    renderer = std::make_unique<Renderer>(render.generate_renderer());
                    render_future = render_thread(
                        args,
                        button_states,
                        key_configurations,
                        physics_scenes,
                        renderable_scenes,
                        load_scene_finished,
                        *renderer,
                        scene_config,
                        menu_logic);
                }
                std::function<void()> exit = [&render](){
                    render.request_window_close();
                };
                render.window().set_title(main_scene_filename);
                load_scene.reset(new LoadScene(
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
                    ui_focuses,
                    users,
                    layout_constraints,
                    gallery,
                    asset_references,
                    translators,
                    physics_scenes,
                    renderable_scenes,
                    window_logic,
                    exit));
                JThread loader_future_guard{loader_thread(
                    args,
                    physics_scenes,
                    renderable_scenes,
                    *load_scene,
                    load_scene_finished,
                    render_delay,
                    velocity_dt)};
                try {
                    main_func(
                        [&f0=ui_focuses[0]](uint32_t c32){
                            std::scoped_lock lock{ f0.edit_mutex };
                            if (f0.edit_focus.has_value()) {
                                auto& value = f0.edit_focus->value;
                                auto c8 = integral_cast<char>(c32);
                                if (std::isalpha(c8) || std::isdigit(c8)) {
                                    value += c8;
                                } else if ((c8 == '\b') && (!value.empty())) {
                                    value = value.substr(0, value.length() - 1);
                                }
                            }
                        },
                        button_states,
                        cursor_states,
                        scroll_wheel_states,
                        args_num_renderings,
                        renderer.get(),
                        input_config,
                        [&window_logic](){ window_logic.handle_events(); });
                } catch (...) {
                    add_unhandled_exception(std::current_exception());
                }
                if (args.has_named_value("--write_loaded_resources")) {
                    scene_node_resources.write_loaded_resources(args.named_value("--write_loaded_resources"));
                }
            }
            ui_focuses.clear_focuses();
            if (auto s = (std::string)next_scene_filename; !s.empty()) {
                main_scene_filename = s;
            } else {
                main_scene_filename = initial_main_scene_filename;
            }
        }

        ui_focuses.try_save();

        // if (!TimeGuard::is_empty(std::this_thread::get_id())) {
        //     lerr() << "write svg";
        //     TimeGuard::write_svg(std::this_thread::get_id(), "/tmp/events.svg");
        // }
    } catch (const CommandLineArgumentError& e) {
        lerr() << e.what();
        return 1;
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        return 1;
    }
    return 0;
}
