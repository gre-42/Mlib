#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/String.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_obj_file <filename ...> "
        "[--scale <scale>] "
        "[--nsamples_msaa <nsamples>] "
        "[--blend_mode {off,continuous,binary}] "
        "[--aggregate_mode {off, once, sorted}] "
        "[--no_cull_faces] "
        "[--wire_frame] "
        "[--render_dt <dt>] "
        "[--width <width>] "
        "[--height <height>] "
        "[--output <file.ppm>] "
        "[--apply_static_lighting] "
        "[--min_num] <min_num> "
        "[--regex] <regex> "
        "[--no_werror] "
        "[--light_ambience <light_ambience>] "
        "[--light_diffusivity <light_diffusivity>] "
        "[--light_specularity <light_specularity>] "
        "[--no_shadows] "
        "[--light_configuration {none, one, circle}]\n"
        "Keys: Left, Right, Up, Down, PgUp, PgDown, Ctrl as modifier",
        {"--no_cull_faces", "--wire_frame", "--no_werror", "--apply_static_lighting", "--no_shadows"},
        {"--scale", "--y", "--nsamples_msaa", "--blend_mode", "--aggregate_mode", "--render_dt", "--width", "--height", "--output", "--min_num", "--regex",
         "--light_ambience", "--light_diffusivity", "--light_specularity", "--light_configuration"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed_atleast(1);

        // Declared as first class to let destructors of other classes succeed.
        size_t num_renderings = SIZE_MAX;
        RenderResults render_results;
        RenderedSceneDescriptor rsd{external_render_pass: {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, time_id: 0, light_resource_id: 0};
        if (args.has_named_value("--output")) {
            render_results.outputs[rsd] = Array<float>{};
        }
        Render2 render2{
            num_renderings,
            &render_results,
            RenderConfig{
                nsamples_msaa: safe_stoi(args.named_value("--nsamples_msaa", "1")),
                cull_faces: !args.has_named("--no_cull_faces"),
                wire_frame: args.has_named("--wire_frame"),
                screen_width: safe_stoi(args.named_value("--width", "640")),
                screen_height: safe_stoi(args.named_value("--height", "480")),
                show_mouse_cursor: true,
                dt: safe_stof(args.named_value("--render_dt", "0.01667"))}};

        render2.print_hardware_info();

        RenderingResources rendering_resources;
        SceneNodeResources scene_node_resources;
        AggregateArrayRenderer small_sorted_aggregate_renderer{&rendering_resources};
        AggregateArrayRenderer large_aggregate_renderer{&rendering_resources};
        Scene scene{
            &small_sorted_aggregate_renderer,
            &large_aggregate_renderer};
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = new SceneNode;
        {
            size_t i = 0;
            for(const std::string& filename : args.unnamed_values()) {
                std::string name = "obj-" + std::to_string(i++);
                scene_node_resources.add_resource(name, std::make_shared<RenderableObjFile>(
                    filename,
                    fixed_zeros<float, 3>(),                                              // position
                    fixed_zeros<float, 3>(),                                              // rotation
                    fixed_full<float, 3>(safe_stof(args.named_value("--scale", "1"))),
                    &rendering_resources,
                    false,                                                                // is_small
                    blend_mode_from_string(args.named_value("--blend_mode", "binary")),
                    false,                                                                // blend_cull_faces
                    args.has_named("--no_shadows") || (light_configuration == "none") ? OccludedType::OFF : OccludedType::LIGHT_MAP_DEPTH,
                    OccluderType::BLACK,
                    true,                                                                 // occluded_by_black
                    aggregate_mode_from_string(args.named_value("--aggregate_mode", "off")),
                    args.has_named("--apply_static_lighting"),
                    !args.has_named("--no_werror")));
                scene_node->set_position({0.f, safe_stoi(args.named_value("--y", "0")), -40.f});
                scene_node_resources.instantiate_renderable(
                    name,
                    name,
                    *scene_node,
                    SceneNodeResourceFilter{
                        min_num: (size_t)safe_stoi(args.named_value("--min_num", "0")),
                        regex: std::regex{args.named_value("--regex", "")}});
            }
        }
        scene.add_root_node("obj", scene_node);

        std::list<Light*> lights;
        SelectedCameras selected_cameras;
        if (light_configuration == "one") {
            scene.add_root_node("light_node0", new SceneNode);
            scene.get_node("light_node0")->set_position({0.f, 50.f, 0.f});
            scene.get_node("light_node0")->set_rotation({-45.f * M_PI / 180.f, 0.f, 0.f});
            lights.push_back(new Light{resource_index: selected_cameras.add_light_node("light_node0"), only_black: false});
            scene.get_node("light_node0")->add_light(lights.back());
            scene.get_node("light_node0")->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
        } else if (light_configuration == "circle") {
            size_t n = 10;
            float r = 50;
            size_t i = 0;
            for (float a : linspace<float>(0, 2 * M_PI, n).flat_iterable()) {
                std::string name = "light" + std::to_string(i++);
                scene.add_root_node(name, new SceneNode);
                scene.get_node(name)->set_position({float(r * sin(a)), 50.f, float(r * cos(a))});
                scene.get_node(name)->set_rotation(matrix_2_tait_bryan_angles(lookat(
                    scene.get_node(name)->position(),
                    scene.get_node("obj")->position())));
                lights.push_back(new Light{resource_index: selected_cameras.add_light_node(name), only_black: false});
                scene.get_node(name)->add_light(lights.back());
                scene.get_node(name)->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
                lights.back()->ambience *= 2.f / n;
                lights.back()->diffusivity *= 2.f / n;
                lights.back()->specularity *= 2.f / n;
            }
        } else if (light_configuration != "none") {
            throw std::runtime_error("Unknown light configuration");
        }
        
        scene.add_root_node("follower_camera", new SceneNode);
        scene.get_node("follower_camera")->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
        
        // scene.print();
        std::list<Focus> focus = {Focus::SCENE};
        ButtonStates button_states;
        StandardCameraLogic standard_camera_logic{scene, selected_cameras};
        StandardRenderLogic standard_render_logic{scene, standard_camera_logic};
        FlyingCameraUserClass user_object{
            button_states: button_states,
            cameras: selected_cameras,
            focus: focus,
            physics_set_fps: nullptr};
        auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
            render2.window(),
            button_states,
            scene,
            user_object,
            true,               // fly
            true);              // rotate
        auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);
        std::list<std::shared_ptr<LightmapLogic>> lightmap_logics;
        for(const Light* l : lights) {
            lightmap_logics.push_back(std::make_shared<LightmapLogic>(
                *read_pixels_logic,
                rendering_resources,
                LightmapUpdateCycle::ALWAYS,
                l->resource_index,
                "",                           // black_node_name
                true));                       // with_depth_texture
        }

        RenderLogics render_logics;
        render_logics.append(nullptr, flying_camera_logic);
        for(const auto& l : lightmap_logics) {
            render_logics.append(nullptr, l);
        }
        render_logics.append(nullptr, read_pixels_logic);

        std::shared_mutex mutex;
        render2(
            render_logics,
            mutex,
            SceneGraphConfig{});
        if (args.has_named_value("--output")) {
            PpmImage::from_float_rgb(render_results.outputs.at(rsd)).save_to_file(args.named_value("--output"));
        }
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
