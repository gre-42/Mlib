#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Time/Fps/Fixed_Time_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: apply_normal_map\n"
        "--color <color>\n"
        "[--histogram <color>]\n"
        "--normal <normal>\n"
        "[--width <width>]\n"
        "[--height <height>]\n"
        "[--uv_scale <uv_scale>]\n"
        "[--output <output>]\n"
        "--light_x <x>\n"
        "--light_y <y>\n"
        "--light_z <z>\n"
        "--light_angle_x <x>\n"
        "--light_angle_y <y>\n"
        "--light_angle_z <z>\n"
        "--light_configuration {one, shifted_circle, circle}",
        {},
        {"--color",
         "--histogram",
         "--normal",
         "--width",
         "--height",
         "--uv_scale",
         "--output",
         "--light_x",
         "--light_y",
         "--light_z",
         "--light_angle_x",
         "--light_angle_y",
         "--light_angle_z",
         "--light_configuration"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

        // Declared as first class to let destructors of other classes succeed.
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderResults render_results;
        RenderedSceneDescriptor rsd;
        if (args.has_named_value("--output")) {
            render_results.outputs[rsd] = {};
        }
        auto in_color = StbImage3::load_from_file(args.named_value("--color"));
        RenderConfig render_config{
            .windowed_width = args.has_named_value("--width") ? safe_stoi(args.named_value("--width")) : (int)in_color.shape(1),
            .windowed_height = args.has_named_value("--height") ? safe_stoi(args.named_value("--height")) : (int)in_color.shape(0)};
        InputConfig input_config;
        FixedTimeSleeper sleeper{ safe_stof(args.named_value("--sleep_dt", "0.01667")) };
        SetFps set_fps{ &sleeper };
        Render render{
            render_config,
            input_config,
            num_renderings,
            set_fps,
            []() { return std::chrono::steady_clock::now(); },
            &render_results };

        render.print_hardware_info();

        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .trail_resources = trail_resources,
            .rendering_resources = rendering_resources,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };
        DeleteNodeMutex delete_node_mutex;
        Scene scene{ "main_scene", delete_node_mutex };
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = make_unique_scene_node();
        {
            TriangleList<float> tl{
                "tl",
                Material{
                    .textures_color{ BlendMapTexture{.texture_descriptor = TextureDescriptor{
                        .color = ColormapWithModifiers{
                            .filename = VariableAndHash{args.named_value("--color")},
                            .histogram = args.named_value("--histogram", ""),
                            .color_mode = ColorMode::RGB}.compute_hash(),
                        .normal = ColormapWithModifiers{
                            .filename = VariableAndHash{args.named_value("--normal")},
                            .color_mode = ColorMode::RGB}.compute_hash()}}}
                    },
                Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE } };
            tl.draw_rectangle_wo_normals(
                FixedArray<float, 3>{-1.f, -1.f, -10.f},
                FixedArray<float, 3>{1.f, -1.f, -10.f},
                FixedArray<float, 3>{1.f, 1.f, -10.f},
                FixedArray<float, 3>{-1.f, 1.f, -10.f},
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                FixedArray<float, 2>{0.f, 0.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{1.f, 0.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{1.f, 1.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{0.f, 1.f} * safe_stof(args.named_value("--uv_scale", "1")));
            auto cva = std::make_shared<ColoredVertexArrayResource>(tl.triangle_array());
            scene_node_resources.add_resource("tl", cva);
            scene_node_resources.instantiate_child_renderable(
                "tl",
                ChildInstantiationOptions{
                    .instance_name = VariableAndHash<std::string>{ "tl" },
                    .scene_node = scene_node.ref(DP_LOC),
                    .interpolation_mode = PoseInterpolationMode::DISABLED,
                    .renderable_resource_filter = RenderableResourceFilter{}});
        }
        scene.auto_add_root_node("obj", std::move(scene_node), RenderingDynamics::STATIC);

        std::list<Light*> lights;
        if (light_configuration == "one") {
            scene.add_root_node(
                "light_node0",
                make_unique_scene_node(
                    FixedArray<ScenePos, 3>{
                        safe_stox<ScenePos>(args.named_value("--light_x", "0")),
                        safe_stox<ScenePos>(args.named_value("--light_y", "50")),
                        safe_stox<ScenePos>(args.named_value("--light_z", "0"))},
                    FixedArray<float, 3>{
                        safe_stof(args.named_value("--light_angle_x", "-45")) * degrees,
                        safe_stof(args.named_value("--light_angle_y", "0")) * degrees,
                        safe_stof(args.named_value("--light_angle_z", "0")) * degrees},
                    1.f),
                RenderingDynamics::STATIC,
                RenderingStrategies::OBJECT);
            auto light = std::make_unique<Light>(Light{
                .shadow_render_pass = ExternalRenderPassType::NONE});
            lights.push_back(light.get());
            scene.get_node("light_node0", DP_LOC)->add_light(std::move(light));
        } else if (light_configuration == "circle" || light_configuration == "shifted_circle") {
            size_t n = 10;
            float r = 50;
            FixedArray<float, 3> center = uninitialized;
            if (light_configuration == "circle") {
                center = {0.f, 10.f, 0.f};
            } else if (light_configuration == "shifted_circle") {
                center = {-50.f, 50.f, -20.f};
            } else {
                throw std::runtime_error("Unknown light configuration");
            }
            for (const auto& [i, a] : enumerate(Linspace<float>(0.f, 2.f * float(M_PI), n))) {
                std::string name = "light" + std::to_string(i);
                auto R = gl_lookat_absolute(
                    scene.get_node(name, DP_LOC)->position(),
                    scene.get_node("obj", DP_LOC)->position());
                if (!R.has_value()) {
                    THROW_OR_ABORT("Lookat failed for light " + std::to_string(i));
                }
                scene.add_root_node(
                    name,
                    make_unique_scene_node(
                        FixedArray<ScenePos, 3>{
                            ScenePos(r * std::cos(a)) + center(0),
                            center(1),
                            ScenePos(r * std::sin(a)) + center(2)},
                        matrix_2_tait_bryan_angles(*R).casted<float>(),
                        1.f),
                    RenderingDynamics::STATIC,
                    RenderingStrategies::OBJECT);
                auto light = std::make_unique<Light>(Light{
                    .shadow_render_pass = ExternalRenderPassType::NONE});
                lights.push_back(light.get());
                scene.get_node(name, DP_LOC)->add_light(std::move(light));
                lights.back()->ambient *= 2.f / float(n);
                lights.back()->diffuse *= 2.f / float(n);
                lights.back()->specular *= 2.f / float(n);
            }
        } else if (light_configuration != "none") {
            throw std::runtime_error("Unknown light configuration");
        }
        
        scene.add_root_node(
            "follower_camera",
            make_unique_scene_node(),
            RenderingDynamics::MOVING,
            RenderingStrategies::OBJECT);
        scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<OrthoCamera>(
            OrthoCameraConfig{.left_plane = -1, .right_plane = 1, .bottom_plane = -1, .top_plane = 1},
            OrthoCamera::Postprocessing::ENABLED));
        
        // scene.print();
        SelectedCameras selected_cameras{scene};
        StandardCameraLogic standard_camera_logic{
            scene,
            selected_cameras};
        StandardRenderLogic standard_render_logic{
            scene,
            standard_camera_logic,
            {1.f, 0.f, 1.f},
            ClearMode::COLOR_AND_DEPTH};
        ButtonStates button_states;
        LockableKeyConfigurations key_configurations;
        ReadPixelsLogic read_pixels_logic{
            standard_render_logic,
            button_states,
            key_configurations,
            ReadPixelsRole::NONE };

        render.render(
            read_pixels_logic,
            []() {},
            SceneGraphConfig());
        if (args.has_named_value("--output")) {
            const Array<float>& array = render_results.outputs.at(rsd).rgb;
            if (!array.initialized()) {
                throw std::runtime_error("Rendered scene descriptor not initialized");
            }
            StbImage3::from_float_rgb(array).save_to_file(args.named_value("--output"));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
