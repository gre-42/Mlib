#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_scene_file scene.scn\n"
        "    [--wire_frame]\n"
        "    [--no_cull_faces]\n"
        "    [--fly]\n"
        "    [--rotate]\n"
        "    [--swap_interval <interval>]\n"
        "    [--nsamples_msaa <nsamples>]\n"
        "    [--window_maximized]\n"
        "    [--max_distance_small <distance>]\n"
        "    [--aggregate_update_interval <interval>]\n"
        "    [--screen_width <width>]\n"
        "    [--screen_height <height>]\n"
        "    [--scene_lightmap_width <width>]\n"
        "    [--scene_lightmap_height <height>]\n"
        "    [--black_lightmap_width <width>]\n"
        "    [--black_lightmap_height <height>]\n"
        "    [--full_screen]\n"
        "    [--no_physics ]\n"
        "    [--physics_dt <dt> ]\n"
        "    [--render_dt <dt> ]\n"
        "    [--print_physics_residual_time]\n"
        "    [--print_render_residual_time]\n"
        "    [--draw_distance_add <value>]\n"
        "    [--damping <x>]\n"
        "    [--stiction_coefficient <x>]\n"
        "    [--friction_coefficient <x>]\n"
        "    [--alpha0 <x>]\n"
        "    [--lateral_stability <x>]\n"
        "    [--max_extra_friction <x>]\n"
        "    [--max_extra_w <x>]\n"
        "    [--longitudinal_friction_steepness <x>]\n"
        "    [--lateral_friction_steepness <x>]\n"
        "    [--no_slip <x>]\n"
        "    [--no_avoid_burnout]\n"
        "    [--wheel_penetration_depth <x>]\n"
        "    [--print_fps]\n"
        "    [--vfx]\n"
        "    [--no_depth_fog]\n"
        "    [--low_pass]\n"
        "    [--high_pass]\n"
        "    [--motion_interpolation]\n"
        "    [--no_render]\n"
        "    [--print_gamepad_buttons]\n"
        "    [--show_mouse_cursor]\n"
        "    [--physics_type {version1, tracking_springs, builtin}]\n"
        "    [--resolve_collision_type {penalty, sequential_pulses}]\n"
        "    [--no_bvh]\n"
        "    [--oversampling]\n"
        "    [--static_radius <r>]\n"
        "    [--print_search_time]\n"
        "    [--verbose]",
        {"--wire_frame",
         "--no_cull_faces",
         "--fly",
         "--rotate",
         "--no_physics",
         "--window_maximized",
         "--full_screen",
         "--print_physics_residual_time",
         "--print_render_residual_time",
         "--print_fps",
         "--no_vfx",
         "--no_depth_fog",
         "--low_pass",
         "--high_pass",
         "--motion_interpolation",
         "--no_render",
         "--print_gamepad_buttons",
         "--show_mouse_cursor",
         "--no_bvh",
         "--no_slip",
         "--no_avoid_burnout",
         "--print_search_time",
         "--verbose"},
        {"--swap_interval",
         "--nsamples_msaa",
         "--max_distance_small",
         "--aggregate_update_interval",
         "--screen_width",
         "--screen_height",
         "--scene_lightmap_width",
         "--scene_lightmap_height",
         "--black_lightmap_width",
         "--black_lightmap_height",
         "--static_radius",
         "--physics_dt",
         "--physics_type",
         "--resolve_collision_type",
         "--oversampling",
         "--render_dt",
         "--damping",
         "--stiction_coefficient",
         "--friction_coefficient",
         "--alpha0",
         "--lateral_stability",
         "--max_extra_w",
         "--max_extra_friction",
         "--longitudinal_friction_steepness",
         "--lateral_friction_steepness",
         "--wheel_penetration_depth",
         "--draw_distance_add"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(1);
        std::string main_scene_filename = args.unnamed_value(0);

        size_t num_renderings;
        RenderConfig render_config{
            .nsamples_msaa = safe_stoi(args.named_value("--nsamples_msaa", "2")),
            .cull_faces = !args.has_named("--no_cull_faces"),
            .wire_frame = args.has_named("--wire_frame"),
            .window_title = main_scene_filename,
            .screen_width = safe_stoi(args.named_value("--screen_width", "640")),
            .screen_height = safe_stoi(args.named_value("--screen_height", "480")),
            .scene_lightmap_width = safe_stoi(args.named_value("--scene_lightmap_width", "2048")),
            .scene_lightmap_height = safe_stoi(args.named_value("--scene_lightmap_height", "2048")),
            .black_lightmap_width = safe_stoi(args.named_value("--black_lightmap_width", "1024")),
            .black_lightmap_height = safe_stoi(args.named_value("--black_lightmap_height", "1024")),
            .motion_interpolation = args.has_named("--motion_interpolation"),
            .full_screen = args.has_named("--full_screen"),
            .window_maximized = args.has_named("--window_maximized"),
            .show_mouse_cursor = args.has_named("--show_mouse_cursor"),
            .swap_interval = safe_stoi(args.named_value("--swap_interval", "1")),
            .background_color = {0.68, 0.85, 1},
            .print_fps = args.has_named("--print_fps"),
            .print_residual_time = args.has_named("--print_render_residual_time"),
            .dt = safe_stof(args.named_value("--render_dt", "0.01667")),
            .draw_distance_add = safe_stof(args.named_value("--draw_distance_add", "NAN"))};
        // Declared as first class to let destructors of other classes succeed.
        Render2 render2{
            num_renderings,
            nullptr,
            render_config};
        
        render2.print_hardware_info();

        ButtonStates button_states;
        UiFocus ui_focus = UiFocus{focus: {Focus::SCENE}};
        SubstitutionString substitutions;
        std::map<std::string, size_t> selection_ids;
        FifoLog fifo_log{10 * 1000};

        while (!render2.window_should_close()) {
            // TimeGuard::initialize(5000);
            num_renderings = SIZE_MAX;
            ui_focus.n_submenus = 0;

            SceneConfig scene_config;

            scene_config.render_config = render_config;

            scene_config.scene_graph_config = SceneGraphConfig{
                .min_distance_small = 1,
                .max_distance_small = safe_stof(args.named_value("--max_distance_small", "1000")),
                .aggregate_update_interval = safe_stoz(args.named_value("--aggregate_update_interval", "100"))};

            scene_config.physics_engine_config = PhysicsEngineConfig{
                .dt = safe_stof(args.named_value("--physics_dt", "0.01667")),
                .print_residual_time = args.has_named("--print_physics_residual_time"),
                .damping = safe_stof(args.named_value("--damping", "0")),
                .stiction_coefficient = safe_stof(args.named_value("--stiction_coefficient", "0.5")),
                .friction_coefficient = safe_stof(args.named_value("--friction_coefficient", "0.5")),
                .alpha0 = safe_stof(args.named_value("--alpha0", "0.1")),
                .avoid_burnout = !args.has_named("--no_avoid_burnout"),
                .no_slip = args.has_named("--no_slip"),
                .lateral_stability = safe_stof(args.named_value("--lateral_stability", "1")),
                .max_extra_friction = safe_stof(args.named_value("--max_extra_friction", "0")),
                .max_extra_w = safe_stof(args.named_value("--max_extra_w", "0")),
                .longitudinal_friction_steepness = safe_stof(args.named_value("--longitudinal_friction_steepness", "5")),
                .lateral_friction_steepness = safe_stof(args.named_value("--lateral_friction_steepness", "7")),
                .wheel_penetration_depth = safe_stof(args.named_value("--wheel_penetration_depth", "0.25")),
                .static_radius = safe_stof(args.named_value("--static_radius", "100")),
                .physics_type = physics_type_from_string(args.named_value("--physics_type", "builtin")),
                .resolve_collision_type = resolve_collission_type_from_string(args.named_value("--resolve_collision_type", "sequential_pulses")),
                .bvh = !args.has_named("--no_bvh"),
                .oversampling = safe_stoz(args.named_value("--oversampling", "2"))};

            SceneNodeResources scene_node_resources;
            RenderingResources rendering_resources{scene_node_resources};
            RenderableSceneConfig config{
                .fly = args.has_named("--fly"),
                .rotate = args.has_named("--rotate"),
                .print_gamepad_buttons = args.has_named("--print_gamepad_buttons"),
                .depth_fog = !args.has_named("--no_depth_fog"),
                .low_pass = args.has_named("--low_pass"),
                .high_pass = args.has_named("--high_pass"),
                .vfx = !args.has_named("--no_vfx")
            };
            RenderableScene renderable_scene{
                scene_node_resources,
                rendering_resources,
                scene_config,
                button_states,
                ui_focus,
                selection_ids,
                render2,
                config};

            std::string next_scene_filename;
            renderable_scene.load_scene_file(
                main_scene_filename,
                next_scene_filename,
                num_renderings,
                args.has_named("--verbose"));

            if (args.has_named("--print_search_time")) {
                renderable_scene.print_physics_engine_search_time();
                return 0;
            }

            if (!args.has_named("--no_physics")) {
                renderable_scene.start_physics_loop();
            }

            if (args.has_named("--no_render")) {
                std::cout << "Press enter to exit" << std::endl;
                std::cin.get();
            } else {
                render2(
                    renderable_scene,
                    scene_config.scene_graph_config);
            }
            if (!render2.window_should_close()) {
                ui_focus.focus = {Focus::SCENE, Focus::LOADING};
                num_renderings = 1;
                render2(
                    renderable_scene,
                    scene_config.scene_graph_config);
                ui_focus.focus.pop_back();
            }

            renderable_scene.stop_and_join();
            main_scene_filename = next_scene_filename;
        }

    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (!TimeGuard::is_empty()) {
        TimeGuard::write_svg("/tmp/events.svg");
    }
    return 0;
}
