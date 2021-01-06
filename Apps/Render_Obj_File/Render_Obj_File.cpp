#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Load_Bvh.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
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
#include <Mlib/Render/Renderables/Renderable_Mhx2_File.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/String.hpp>
#include <vector>

using namespace Mlib;

void add_reference_bone(
    const Bone& b,
    SceneNode* parent_node,
    SceneNodeResources& scene_node_resources)
{
    SceneNode* bone_node = new SceneNode;
    bone_node->set_position(b.initial_absolute_transformation.offset());
    bone_node->set_rotation(b.initial_absolute_transformation.quaternion().to_tait_bryan_angles());
    scene_node_resources.instantiate_renderable(
        "reference_bone",
        "reference_bone",
        *bone_node,
        SceneNodeResourceFilter{});
    parent_node->add_child("reference_bone" + std::to_string(b.index), bone_node);
    for (const auto& c : b.children) {
        add_reference_bone(*c, parent_node, scene_node_resources);
    }
}

void add_bone_frame(
    const Bone& b,
    const std::vector<OffsetAndQuaternion<float>>& frame,
    SceneNode* parent_node,
    SceneNodeResources& scene_node_resources)
{
    if (b.index >= frame.size()) {
        throw std::runtime_error("Frame index too large");
    }
    SceneNode* bone_node = new SceneNode;
    bone_node->set_position(frame.at(b.index).offset());
    bone_node->set_rotation(frame.at(b.index).quaternion().to_tait_bryan_angles());
    scene_node_resources.instantiate_renderable(
        "frame_bone",
        "frame_bone",
        *bone_node,
        SceneNodeResourceFilter{});
    parent_node->add_child("frame_bone" + std::to_string(b.index), bone_node);
    for (const auto& c : b.children) {
        add_bone_frame(*c, frame, bone_node, scene_node_resources);
    }
}

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_obj_file <filename ...> "
        "[--bvh <filename>] "
        "[--animation_frame <id>] "
        "[--bone_frame <id>] "
        "[--hide_object] "
        "[--scale <scale>] "
        "[--node_scale <scale>] "
        "[--bvh_scale <scale>] "
        "[--bvh_demean] "
        "[--bvh_rotation_0] "
        "[--bvh_rotation_1] "
        "[--bvh_rotation_2] "
        "[--bvh_trafo] "
        "[--reference_bone <filename>] "
        "[--frame_bone <filename>] "
        "[--bone_scale <scale>] "
        "[--x <x>] "
        "[--y <y>] "
        "[--z <z>] "
        "[--angle_x <angle_x>] "
        "[--angle_y <angle_y>] "
        "[--angle_z <angle_z>] "
        "[--nsamples_msaa <nsamples>] "
        "[--blend_mode {off,continuous,binary,binary_add}] "
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
        "[--color_gradient_min_x] <value> "
        "[--color_gradient_max_x] <value> "
        "[--color_gradient_min_c] <value> "
        "[--color_gradient_max_c] <value> "
        "[--color_radial_center_x] <value> "
        "[--color_radial_center_y] <value> "
        "[--color_radial_center_z] <value> "
        "[--color_radial_min_r] <value> "
        "[--color_radial_max_r] <value> "
        "[--color_radial_min_c] <value> "
        "[--color_radial_max_c] <value> "
        "[--color_cone_x] <value> "
        "[--color_cone_z] <value> "
        "[--color_cone_bottom] <value> "
        "[--color_cone_top] <value> "
        "[--color_cone_min_r] <value> "
        "[--color_cone_max_r] <value> "
        "[--color_cone_min_c] <value> "
        "[--color_cone_max_c] <value> "
        "[--color_r] <value> "
        "[--color_g] <value> "
        "[--color_b] <value> "
        "[--background_r] <value> "
        "[--background_g] <value> "
        "[--background_b] <value> "
        "[--background_light_ambience <background_light_ambience>] "
        "[--no_shadows] "
        "[--light_configuration {none, one, shifted_circle, circle}]\n"
        "Keys: Left, Right, Up, Down, PgUp, PgDown, Ctrl as modifier",
        {"--hide_object",
         "--no_cull_faces",
         "--wire_frame",
         "--no_werror",
         "--apply_static_lighting",
         "--no_shadows",
         "--bvh_demean"},
        {"--bvh",
         "--bvh_rotation_0",
         "--bvh_rotation_1",
         "--bvh_rotation_2",
         "--bvh_trafo",
         "--reference_bone",
         "--frame_bone",
         "--bone_scale",
         "--animation_frame",
         "--bone_frame",
         "--scale",
         "--node_scale",
         "--bvh_scale",
         "--x",
         "--y",
         "--z",
         "--angle_x",
         "--angle_y",
         "--angle_z",
        "--nsamples_msaa",
        "--blend_mode",
        "--aggregate_mode",
        "--render_dt",
        "--width",
        "--height",
        "--output",
        "--min_num",
        "--regex",
         "--background_light_ambience",
         "--light_configuration",
         "--color_gradient_min_x",
         "--color_gradient_max_x",
         "--color_gradient_min_c",
         "--color_gradient_max_c",
         "--color_radial_center_x",
         "--color_radial_center_y",
         "--color_radial_center_z",
         "--color_radial_min_r",
         "--color_radial_max_r",
         "--color_radial_min_c",
         "--color_radial_max_c",
         "--color_cone_x",
         "--color_cone_z",
         "--color_cone_bottom",
         "--color_cone_top",
         "--color_cone_min_r",
         "--color_cone_max_r",
         "--color_cone_min_c",
         "--color_cone_max_c",
         "--color_r",
         "--color_g",
         "--color_b",
         "--background_r",
         "--background_g",
         "--background_b"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed_atleast(1);

        // Declared as first class to let destructors of other classes succeed.
        size_t num_renderings = SIZE_MAX;
        RenderResults render_results;
        RenderedSceneDescriptor rsd{.external_render_pass = {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = 0, .light_node_name = ""};
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
                background_color: {
                    safe_stof(args.named_value("--background_r", "1")),
                    safe_stof(args.named_value("--background_g", "0")),
                    safe_stof(args.named_value("--background_b", "1"))},
                dt: safe_stof(args.named_value("--render_dt", "0.01667"))}};

        render2.print_hardware_info();

        SceneNodeResources scene_node_resources;
        RenderingResources rendering_resources{scene_node_resources};
        AggregateArrayRenderer small_sorted_aggregate_renderer{rendering_resources};
        AggregateArrayRenderer large_aggregate_renderer{rendering_resources};
        Scene scene{
            &small_sorted_aggregate_renderer,
            &large_aggregate_renderer};
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = new SceneNode;
        {
            size_t i = 0;
            for (const std::string& filename : args.unnamed_values()) {
                std::string name = "obj-" + std::to_string(i++);
                LoadMeshConfig cfg{
                    .position = fixed_zeros<float, 3>(),
                    .rotation = fixed_zeros<float, 3>(),
                    .scale = fixed_full<float, 3>(safe_stof(args.named_value("--scale", "1"))),
                    .is_small = false,
                    .blend_mode = blend_mode_from_string(args.named_value("--blend_mode", "binary")),
                    .cull_faces = !args.has_named("--no_cull_faces"),
                    .occluded_type = args.has_named("--no_shadows") || (light_configuration == "none") ? OccludedType::OFF : OccludedType::LIGHT_MAP_DEPTH,
                    .occluder_type = OccluderType::BLACK,
                    .occluded_by_black = true,
                    .aggregate_mode = aggregate_mode_from_string(args.named_value("--aggregate_mode", "off")),
                    .transformation_mode = TransformationMode::ALL,
                    .apply_static_lighting = args.has_named("--apply_static_lighting"),
                    .werror = !args.has_named("--no_werror")};
                if (filename.ends_with(".obj")) {
                    scene_node_resources.add_resource(name, std::make_shared<RenderableObjFile>(
                        filename,
                        cfg,
                        rendering_resources));
                } else if (filename.ends_with(".mhx2")) {
                    auto rmhx2 = std::make_shared<RenderableMhx2File>(
                        filename,
                        cfg,
                        rendering_resources);
                    scene_node_resources.add_resource(name, rmhx2);
                    scene_node->set_style(new Style{.selector = std::regex{""}, .animation_frame = {.name = "anim"}});
                    LoadMeshConfig bone_cfg{
                        .position = fixed_zeros<float, 3>(),
                        .rotation = fixed_zeros<float, 3>(),
                        .scale = fixed_full<float, 3>(safe_stof(args.named_value("--bone_scale", "1"))),
                        .is_small = false,
                        .blend_mode = BlendMode::OFF,
                        .cull_faces = !args.has_named("--no_cull_faces"),
                        .occluded_type = OccludedType::OFF,
                        .occluder_type = OccluderType::OFF,
                        .occluded_by_black = false,
                        .aggregate_mode = AggregateMode::OFF,
                        .transformation_mode = TransformationMode::ALL,
                        .apply_static_lighting = false,
                        .werror = !args.has_named("--no_werror")};
                    if (args.has_named_value("--reference_bone")) {
                        scene_node_resources.add_resource("reference_bone", std::make_shared<RenderableObjFile>(
                            args.named_value("--reference_bone"),
                            bone_cfg,
                            rendering_resources));
                        add_reference_bone(rmhx2->skeleton(), scene_node, scene_node_resources);
                    }
                    if (args.has_named_value("--bvh")) {
                        auto bvh = std::make_shared<BvhLoader>(
                            args.named_value("--bvh"),
                            BvhConfig{
                                .demean = args.has_named_value("--bvh_demean") ? safe_stob(args.named_value("--bvh_demean")) : blender_bvh_config.demean,
                                .scale = args.has_named_value("--bvh_scale") ? safe_stof(args.named_value("--bvh_scale")) : blender_bvh_config.scale,
                                .rotation_order = FixedArray<size_t, 3>{
                                    args.has_named_value("--bvh_rotation_0") ? safe_stoz(args.named_value("--bvh_rotation_0")) : blender_bvh_config.rotation_order(0),
                                    args.has_named_value("--bvh_rotation_1") ? safe_stoz(args.named_value("--bvh_rotation_1")) : blender_bvh_config.rotation_order(1),
                                    args.has_named_value("--bvh_rotation_2") ? safe_stoz(args.named_value("--bvh_rotation_2")) : blender_bvh_config.rotation_order(2)},
                                .parameter_transformation = args.has_named_value("--bvh_trafo")
                                    ? get_parameter_transformation(args.named_value("--bvh_trafo"))
                                    : blender_bvh_config.parameter_transformation});
                        scene_node_resources.add_bvh_loader("anim", bvh);
                        if (args.has_named_value("--frame_bone")) {
                            float bone_frame = safe_stof(args.named_value("--bone_frame"));
                            scene_node_resources.add_resource("frame_bone", std::make_shared<RenderableObjFile>(
                                args.named_value("--frame_bone"),
                                bone_cfg,
                                rendering_resources));
                            add_bone_frame(
                                rmhx2->skeleton(),
                                rmhx2->vectorize_joint_poses(bvh->get_interpolated_frame(bone_frame)),
                                scene_node,
                                scene_node_resources);
                        }
                        // This invalidates the bone weights and clears the skeleton => must be after "add_bone_frame"
                        if (args.has_named_value("--animation_frame")) {
                            float animation_frame = safe_stof(args.named_value("--animation_frame"));
                            scene_node_resources.set_relative_joint_poses(name, bvh->get_interpolated_frame(animation_frame));
                        }
                    }
                } else {
                    throw std::runtime_error("File has unknown extension: " + filename);
                }
                scene_node->set_position({
                    safe_stof(args.named_value("--x", "0")),
                    safe_stof(args.named_value("--y", "0")),
                    safe_stof(args.named_value("--z", "-40"))});
                scene_node->set_rotation({
                    safe_stof(args.named_value("--angle_x", "0")) / 180.f * float(M_PI),
                    safe_stof(args.named_value("--angle_y", "0")) / 180.f * float(M_PI),
                    safe_stof(args.named_value("--angle_z", "0")) / 180.f * float(M_PI)});
                scene_node->set_scale(safe_stof(args.named_value("--node_scale", "1")));
                if (!args.has_named("--hide_object")) {
                    scene_node_resources.instantiate_renderable(
                        name,
                        name,
                        *scene_node,
                        SceneNodeResourceFilter{
                            min_num: safe_stoz(args.named_value("--min_num", "0")),
                            regex: std::regex{args.named_value("--regex", "")}});
                }
                if (args.has_named_value("--color_gradient_min_x") || args.has_named_value("--color_gradient_max_x")) {
                    Interp<float> interp{
                        {safe_stof(args.named_value("--color_gradient_min_x")),
                         safe_stof(args.named_value("--color_gradient_max_x"))},
                        {safe_stof(args.named_value("--color_gradient_min_c")),
                         safe_stof(args.named_value("--color_gradient_max_c"))},
                        OutOfRangeBehavior::CLAMP};
                    for (auto& m : scene_node_resources.get_triangle_meshes(name)) {
                        for (auto& t : m->triangles) {
                            for (auto& v : t.flat_iterable()) {
                                v.color = interp(v.position(0));
                            }
                        }
                    }
                }
                if (args.has_named_value("--color_radial_min_r") || args.has_named_value("--color_radial_max_r")) {
                    Interp<float> interp{
                        {safe_stof(args.named_value("--color_radial_min_r")),
                         safe_stof(args.named_value("--color_radial_max_r"))},
                        {safe_stof(args.named_value("--color_radial_min_c")),
                         safe_stof(args.named_value("--color_radial_max_c"))},
                        OutOfRangeBehavior::CLAMP};
                    FixedArray<float, 3> center{
                        safe_stof(args.named_value("--color_radial_center_x", "0")),
                        safe_stof(args.named_value("--color_radial_center_y", "0")),
                        safe_stof(args.named_value("--color_radial_center_z", "0"))};
                    for (auto& m : scene_node_resources.get_triangle_meshes(name)) {
                        for (auto& t : m->triangles) {
                            for (auto& v : t.flat_iterable()) {
                                v.color = interp(std::sqrt(sum(squared(v.position - center))));
                            }
                        }
                    }
                }
                if (args.has_named_value("--color_cone_min_r") || args.has_named_value("--color_cone_max_r")) {
                    Interp<float> interp{
                        {safe_stof(args.named_value("--color_cone_min_r")),
                         safe_stof(args.named_value("--color_cone_max_r"))},
                        {safe_stof(args.named_value("--color_cone_min_c")),
                         safe_stof(args.named_value("--color_cone_max_c"))},
                        OutOfRangeBehavior::CLAMP};
                    float bottom = safe_stof(args.named_value("--color_cone_bottom", "0"));
                    float top = safe_stof(args.named_value("--color_cone_top"));
                    float cx = safe_stof(args.named_value("--color_cone_x", "0"));
                    float cz = safe_stof(args.named_value("--color_cone_z", "0"));
                    for (auto& m : scene_node_resources.get_triangle_meshes(name)) {
                        for (auto& t : m->triangles) {
                            for (auto& v : t.flat_iterable()) {
                                float r = std::sqrt(squared(v.position(0) - cx) + squared(v.position(2) - cz));
                                float h = (top - v.position(1)) / (top - bottom);
                                v.color = interp(r / h);
                            }
                        }
                    }
                }
                FixedArray<float, 3> color{
                    safe_stof(args.named_value("--color_r", "-1")),
                    safe_stof(args.named_value("--color_g", "-1")),
                    safe_stof(args.named_value("--color_b", "-1"))};
                if (any(color != -1.f)) {
                    for (auto& m : scene_node_resources.get_triangle_meshes(name)) {
                        for (auto& t : m->triangles) {
                            for (auto& v : t.flat_iterable()) {
                                v.color = maximum(color, 0.f);
                            }
                        }
                    }
                }
            }
        }
        scene.add_root_node("obj", scene_node);

        std::list<Light*> lights;
        SelectedCameras selected_cameras{scene};
        if (light_configuration == "one") {
            scene.add_root_node("light_node0", new SceneNode);
            scene.get_node("light_node0")->set_position({0.f, 50.f, 0.f});
            scene.get_node("light_node0")->set_rotation({-45.f * M_PI / 180.f, 0.f, 0.f});
            lights.push_back(new Light{.node_name = "light_node0", .only_black = false, .shadow = true});
            scene.get_node("light_node0")->add_light(lights.back());
            scene.get_node("light_node0")->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
        } else if (light_configuration == "circle" || light_configuration == "shifted_circle") {
            size_t n = 10;
            float r = 50;
            size_t i = 0;
            bool with_diffusivity = true;
            FixedArray<float, 3> center;
            if (light_configuration == "circle") {
                center = {0, 10, 0};
            } else if (light_configuration == "shifted_circle") {
                center = {-50, 50, -20};
            } else {
                throw std::runtime_error("Unknown light configuration");
            }
            for (float a : linspace<float>(0, 2 * M_PI, n).flat_iterable()) {
                std::string name = "light" + std::to_string(i++);
                scene.add_root_node(name, new SceneNode);
                scene.get_node(name)->set_position({float(r * cos(a)) + center(0), center(1), float(r * sin(a)) + center(2)});
                scene.get_node(name)->set_rotation(matrix_2_tait_bryan_angles(lookat(
                    scene.get_node(name)->position(),
                    scene.get_node("obj")->position())));
                lights.push_back(new Light{.node_name = name, .only_black = false, .shadow = true});
                scene.get_node(name)->add_light(lights.back());
                scene.get_node(name)->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
                lights.back()->ambience *= 2.f / (n * (1 + with_diffusivity));
                lights.back()->diffusivity = 0;
                lights.back()->specularity = 0;
            }
            if (with_diffusivity) {
                for (float a : linspace<float>(0, 2 * M_PI, n).flat_iterable()) {
                    std::string name = "light_s" + std::to_string(i++);
                    scene.add_root_node(name, new SceneNode);
                    scene.get_node(name)->set_position({float(r * cos(a)) + center(0), center(1), float(r * sin(a)) + center(2)});
                    scene.get_node(name)->set_rotation(matrix_2_tait_bryan_angles(lookat(
                        scene.get_node(name)->position(),
                        scene.get_node("obj")->position())));
                    lights.push_back(new Light{.node_name = name, .shadow = false});
                    scene.get_node(name)->add_light(lights.back());
                    scene.get_node(name)->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
                    lights.back()->ambience = 0;
                    lights.back()->diffusivity /= 2 * n;
                    lights.back()->specularity = 0;
                }
            }
        } else if (light_configuration != "none") {
            throw std::runtime_error("Unknown light configuration");
        }
        if (args.has_named_value("--background_light_ambience")) {
            std::string name = "background_light";
            scene.add_root_node(name, new SceneNode);
            lights.push_back(new Light{.node_name = name, .only_black = false, .shadow = false});
            scene.get_node(name)->add_light(lights.back());
            scene.get_node(name)->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
            lights.back()->ambience = FixedArray<float, 3>{1, 1, 1} * safe_stof(args.named_value("--background_light_ambience"));
            lights.back()->diffusivity = 0;
            lights.back()->specularity = 0;
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
        for (const Light* l : lights) {
            lightmap_logics.push_back(std::make_shared<LightmapLogic>(
                *read_pixels_logic,
                rendering_resources,
                LightmapUpdateCycle::ALWAYS,
                l->node_name,
                "",                           // black_node_name
                true));                       // with_depth_texture
        }

        std::recursive_mutex mutex;
        RenderLogics render_logics{mutex};
        render_logics.append(nullptr, flying_camera_logic);
        for (const auto& l : lightmap_logics) {
            render_logics.append(nullptr, l);
        }
        render_logics.append(nullptr, read_pixels_logic);

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
