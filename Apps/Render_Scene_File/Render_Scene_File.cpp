#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Objects/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Motion_Interpolation_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Physics_Loop.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Set_Fps.hpp>
#include <Mlib/String.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_scene_file scene.scn "
        "[--wire_frame] "
        "[--no_cull_faces] "
        "[--fly] "
        "[--rotate] "
        "[--swap_interval <interval>] "
        "[--nsamples_msaa <nsamples>] "
        "[--window_maximized] "
        "[--max_distance_small <distance>] "
        "[--aggregate_update_interval <interval>] "
        "[--screen_width <width>] "
        "[--screen_height <height>] "
        "[--scene_lightmap_width <width>] "
        "[--scene_lightmap_height <height>] "
        "[--black_lightmap_width <width>] "
        "[--black_lightmap_height <height>] "
        "[--full_screen] "
        "[--no_physics ] "
        "[--physics_dt <dt> ] "
        "[--render_dt <dt> ] "
        "[--print_residual_time] "
        "[--damping <x>]"
        "[--stiction_coefficient <x>] "
        "[--friction_coefficient <x>] "
        "[--print_fps] "
        "[--vfx] "
        "[--no_depth_fog] "
        "[--no_low_pass] "
        "[--motion_interpolation] "
        "[--no_render] "
        "[--print_gamepad_buttons] "
        "[--show_mouse_cursor] "
        "[--verbose]",
        {"--wire_frame",
         "--no_cull_faces",
         "--fly",
         "--rotate",
         "--no_physics",
         "--window_maximized",
         "--full_screen",
         "--print_residual_time",
         "--print_fps",
         "--vfx",
         "--no_depth_fog",
         "--no_low_pass",
         "--motion_interpolation",
         "--no_render",
         "--print_gamepad_buttons",
         "--show_mouse_cursor",
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
         "--physics_dt",
         "--render_dt",
         "--damping",
         "--stiction_coefficient",
         "--friction_coefficient"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(1);
        std::string main_scene_filename = args.unnamed_value(0);

        size_t num_renderings;
        RenderConfig render_config{
            nsamples_msaa: safe_stoi(args.named_value("--nsamples_msaa", "2")),
            cull_faces: !args.has_named("--no_cull_faces"),
            wire_frame: args.has_named("--wire_frame"),
            window_title: main_scene_filename,
            screen_width: safe_stoi(args.named_value("--screen_width", "640")),
            screen_height: safe_stoi(args.named_value("--screen_height", "480")),
            scene_lightmap_width: safe_stoi(args.named_value("--scene_lightmap_width", "2048")),
            scene_lightmap_height: safe_stoi(args.named_value("--scene_lightmap_height", "2048")),
            black_lightmap_width: safe_stoi(args.named_value("--black_lightmap_width", "1024")),
            black_lightmap_height: safe_stoi(args.named_value("--black_lightmap_height", "1024")),
            motion_interpolation: args.has_named("--motion_interpolation"),
            full_screen: args.has_named("--full_screen"),
            window_maximized: args.has_named("--window_maximized"),
            show_mouse_cursor: args.has_named("--show_mouse_cursor"),
            swap_interval: safe_stoi(args.named_value("--swap_interval", "1")),
            background_color: {0.68, 0.85, 1},
            print_fps: args.has_named("--print_fps"),
            dt: safe_stof(args.named_value("--render_dt", "0.01667")) };
        // Declared as first class to let destructors of other classes succeed.
        Render2 render2{
            num_renderings,
            nullptr,
            render_config};
        
        render2.print_hardware_info();

        ButtonStates button_states;
        ButtonPress button_press{button_states};
        SelectedCameras selected_cameras;
        UiFocus ui_focus = UiFocus{focus: {Focus::SCENE}};
        SubstitutionString substitutions;
        SetFps physics_set_fps;
        FlyingCameraUserClass user_object{
            button_states: button_states,
            cameras: selected_cameras,
            focus: ui_focus.focus,
            physics_set_fps: &physics_set_fps};
        std::map<std::string, size_t> selection_ids;

        while (!render2.window_should_close()) {
            num_renderings = SIZE_MAX;
            selected_cameras.light_node_names.clear();
            ui_focus.n_submenus = 0;

            SceneConfig scene_config;

            scene_config.render_config = render_config;

            scene_config.scene_graph_config = SceneGraphConfig{
                min_distance_small: 1,
                max_distance_small: safe_stof(args.named_value("--max_distance_small", "500")),
                aggregate_update_interval: (size_t)std::stoi(args.named_value("--aggregate_update_interval", "100"))};

            scene_config.physics_engine_config = PhysicsEngineConfig{
                dt: safe_stof(args.named_value("--physics_dt", "0.01667")),
                print_residual_time: args.has_named("--print_residual_time"),
                damping: safe_stof(args.named_value("--damping", "0.00091188")),
                stiction_coefficient: safe_stof(args.named_value("--stiction_coefficient", "2")),
                friction_coefficient: safe_stof(args.named_value("--friction_coefficient", "1.6"))};

            RenderingResources rendering_resources;
            SceneNodeResources scene_node_resources;
            AggregateArrayRenderer small_aggregate_array_renderer{&rendering_resources};
            AggregateArrayRenderer large_aggregate_array_renderer{&rendering_resources};
            ArrayInstancesRenderer small_instances_renderer{&rendering_resources};
            ArrayInstancesRenderer large_instances_renderer{&rendering_resources};
            // SceneNode destructors require that physics engine is destroyed after scene,
            // => Create PhysicsEngine before Scene
            PhysicsEngine physics_engine{scene_config.physics_engine_config};
            Scene scene{
                &small_aggregate_array_renderer,
                &large_aggregate_array_renderer,
                &small_instances_renderer,
                &large_instances_renderer};
            GravityEfp gefp{FixedArray<float, 3>{0, -9.8, 0}};
            StandardCameraLogic standard_camera_logic{
                scene,
                selected_cameras};
            auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
                render2.window(),
                button_states,
                scene,
                user_object,
                args.has_named("--fly"),
                args.has_named("--rotate"));
            auto key_bindings = std::make_shared<KeyBindings>(
                button_press,
                args.has_named("--print_gamepad_buttons"),
                selected_cameras,
                ui_focus.focus,
                scene);
            ReadPixelsLogic read_pixels_logic{standard_camera_logic};
            auto dirtmap_logic = std::make_shared<DirtmapLogic>(read_pixels_logic, rendering_resources);
            auto skybox_logic = std::make_shared<SkyboxLogic>(read_pixels_logic, rendering_resources);
            auto motion_interp_logic = std::make_shared<MotionInterpolationLogic>(read_pixels_logic, InterpolationType::OPTICAL_FLOW);
            auto post_processing_logic = std::make_shared<PostProcessingLogic>(
                *skybox_logic,
                !args.has_named("--no_depth_fog"),
                !args.has_named("--no_low_pass"));
            RenderLogics render_logics;
            render_logics.append(nullptr, flying_camera_logic);
            render_logics.append(nullptr, dirtmap_logic);
            render_logics.append(nullptr, args.has_named("--vfx")
                ? post_processing_logic
                : (scene_config.render_config.motion_interpolation ? std::dynamic_pointer_cast<RenderLogic>(motion_interp_logic) : skybox_logic));
            render_logics.append(nullptr, key_bindings);
            physics_engine.add_external_force_provider(&gefp);
            physics_engine.add_external_force_provider(key_bindings.get());

            Players players{physics_engine.advance_times_};

            std::string next_scene_filename;
            LoadScene{}(
                main_scene_filename,
                main_scene_filename,
                next_scene_filename,
                rendering_resources,
                scene_node_resources,
                players,
                scene,
                physics_engine,
                button_press,
                key_bindings->camera_key_bindings_,
                key_bindings->absolute_movable_idle_bindings_,
                key_bindings->absolute_movable_key_bindings_,
                key_bindings->relative_movable_key_bindings_,
                key_bindings->gun_key_bindings_,
                selected_cameras,
                scene_config.camera_config,
                scene_config.physics_engine_config,
                render_logics,
                standard_camera_logic,
                read_pixels_logic,
                *dirtmap_logic,
                *skybox_logic,
                ui_focus,
                substitutions,
                num_renderings,
                selection_ids,
                args.has_named("--verbose"));
            // scene.print();

            std::shared_mutex mutex;

            std::unique_ptr<PhysicsLoop> pl{args.has_named("--no_physics")
                ? nullptr
                : new PhysicsLoop{
                    scene_node_resources,
                    scene,
                    physics_engine,
                    mutex,
                    scene_config.physics_engine_config,
                    physics_set_fps}};

            if (args.has_named("--no_render")) {
                std::cout << "Press enter to exit" << std::endl;
                std::cin.get();
            } else {
                render2(
                    render_logics,
                    mutex,
                    scene_config.scene_graph_config);
            }
            if (!render2.window_should_close()) {
                ui_focus.focus = {Focus::SCENE, Focus::LOADING};
                num_renderings = 1;
                render2(
                    render_logics,
                    mutex,
                    scene_config.scene_graph_config);
                ui_focus.focus.pop_back();
            }

            if (pl != nullptr) {
                pl->stop_and_join();
            }
            main_scene_filename = next_scene_filename;
        }

    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
