#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
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
        Render2 render2{
            render_config,
            num_renderings,
            &render_results};

        render2.print_hardware_info();

        SceneNodeResources scene_node_resources;
        auto rrg = RenderingContextGuard::root(scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0);
        DeleteNodeMutex delete_node_mutex;
        Scene scene{ delete_node_mutex, nullptr };
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = std::make_unique<SceneNode>();
        {
            TriangleList<float> tl{
                "tl",
                Material{
                    .textures{ BlendMapTexture{.texture_descriptor = TextureDescriptor{
                        .color = args.named_value("--color"),
                        .normal = args.named_value("--normal"),
                        .color_mode = ColorMode::RGB,
                        .histogram = args.named_value("--histogram", "")}} }
                    },
                PhysicsMaterial::ATTR_VISIBLE};
            tl.draw_rectangle_wo_normals(
                FixedArray<float, 3>{-1, -1, -10},
                FixedArray<float, 3>{1, -1, -10},
                FixedArray<float, 3>{1, 1, -10},
                FixedArray<float, 3>{-1, 1, -10},
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                FixedArray<float, 2>{0.f, 0.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{1.f, 0.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{1.f, 1.f} * safe_stof(args.named_value("--uv_scale", "1")),
                FixedArray<float, 2>{0.f, 1.f} * safe_stof(args.named_value("--uv_scale", "1")));
            auto cva = std::make_shared<ColoredVertexArrayResource>(tl.triangle_array());
            scene_node_resources.add_resource("tl", cva);
            scene_node_resources.instantiate_renderable(
                "tl",
                InstantiationOptions{
                    .instance_name = "tl",
                    .scene_node = *scene_node,
                    .renderable_resource_filter = RenderableResourceFilter{}});
        }
        scene.add_root_node("obj", std::move(scene_node));

        std::list<Light*> lights;
        if (light_configuration == "one") {
            scene.add_root_node("light_node0", std::make_unique<SceneNode>());
            scene.get_node("light_node0").set_position({
                safe_stof(args.named_value("--light_x", "0")),
                safe_stof(args.named_value("--light_y", "50")),
                safe_stof(args.named_value("--light_z", "0"))});
            scene.get_node("light_node0").set_rotation({
                safe_stof(args.named_value("--light_angle_x", "-45")) * degrees,
                safe_stof(args.named_value("--light_angle_y", "0")) * degrees,
                safe_stof(args.named_value("--light_angle_z", "0")) * degrees});
            auto light = std::make_unique<Light>(Light{
                .shadow_render_pass = ExternalRenderPassType::NONE});
            lights.push_back(light.get());
            scene.get_node("light_node0").add_light(std::move(light));
        } else if (light_configuration == "circle" || light_configuration == "shifted_circle") {
            size_t n = 10;
            float r = 50;
            size_t i = 0;
            FixedArray<float, 3> center;
            if (light_configuration == "circle") {
                center = {0.f, 10.f, 0.f};
            } else if (light_configuration == "shifted_circle") {
                center = {-50.f, 50.f, -20.f};
            } else {
                throw std::runtime_error("Unknown light configuration");
            }
            for (float a : Linspace<float>(0.f, 2.f * float{ M_PI }, n)) {
                std::string name = "light" + std::to_string(i++);
                scene.add_root_node(name, std::make_unique<SceneNode>());
                scene.get_node(name).set_position({float(r * cos(a)) + center(0), center(1), float(r * sin(a)) + center(2)});
                scene.get_node(name).set_rotation(matrix_2_tait_bryan_angles(gl_lookat_absolute(
                    scene.get_node(name).position(),
                    scene.get_node("obj").position())).casted<float>());
                auto light = std::make_unique<Light>(Light{
                    .shadow_render_pass = ExternalRenderPassType::NONE});
                lights.push_back(light.get());
                scene.get_node(name).add_light(std::move(light));
                lights.back()->ambience *= 2.f / n;
                lights.back()->diffusivity *= 2.f / n;
                lights.back()->specularity *= 2.f / n;
            }
        } else if (light_configuration != "none") {
            throw std::runtime_error("Unknown light configuration");
        }
        
        scene.add_root_node("follower_camera", std::make_unique<SceneNode>());
        scene.get_node("follower_camera").set_camera(std::make_unique<OrthoCamera>(
            OrthoCameraConfig{.left_plane = -1, .right_plane = 1, .bottom_plane = -1, .top_plane = 1},
            OrthoCamera::Postprocessing::ENABLED));
        
        // scene.print();
        SelectedCameras selected_cameras{scene};
        StandardCameraLogic standard_camera_logic{
            scene,
            selected_cameras,
            delete_node_mutex};
        StandardRenderLogic standard_render_logic{
            scene,
            standard_camera_logic,
            {1.f, 0.f, 1.f},
            ClearMode::COLOR_AND_DEPTH};
        auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);

        render2.render(
            *read_pixels_logic,
            SceneGraphConfig());
        if (args.has_named_value("--output")) {
            const Array<float>& array = render_results.outputs.at(rsd).rgb;
            if (!array.initialized()) {
                throw std::runtime_error("Rendered scene descriptor not initialized");
            }
            StbImage3::from_float_rgb(array).save_to_file(args.named_value("--output"));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
