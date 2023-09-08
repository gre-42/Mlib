#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At_Aabb.hpp>
#include <Mlib/Geometry/Coordinates/Npixels_For_Dpi.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Least_Common_Multiple.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Modifiers/Merge_Textures.hpp>
#include <Mlib/Render/Modifiers/Merged_Textures_Config.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Move_Scene_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Bvh_File_Resource.hpp>
#include <Mlib/Render/Resources/Kn5_File_Resource.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Compound_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <vector>

using namespace Mlib;

/** Add the refernce bone hierarchy to the parent node.
 *
 * The refernce bones have the coordinates of the T-pose, typically.
 * It is accessed by Bone::initial_absolute_transformation.
 */
void add_reference_bone(
    const Bone& b,
    DanglingRef<SceneNode> parent_node,
    SceneNodeResources& scene_node_resources)
{
    auto bone_node = make_dunique<SceneNode>();
    bone_node->set_position(b.initial_absolute_transformation.offset().casted<double>());
    bone_node->set_rotation(b.initial_absolute_transformation.quaternion().to_tait_bryan_angles());
    scene_node_resources.instantiate_renderable(
        "reference_bone",
        InstantiationOptions{
            .instance_name = "reference_bone",
            .scene_node = bone_node.ref(DP_LOC),
            .renderable_resource_filter = RenderableResourceFilter{}});
    parent_node->add_child("reference_bone" + std::to_string(b.index), std::move(bone_node));
    for (const auto& c : b.children) {
        add_reference_bone(*c, parent_node, scene_node_resources);
    }
}

/** Add a bone frame.
 *
 * The bone frames' coordinates are relative to their parent bones.
 */
void add_bone_frame(
    const Bone& b,
    const std::vector<OffsetAndQuaternion<float, float>>& frame,
    DanglingRef<SceneNode> parent_node,
    SceneNodeResources& scene_node_resources)
{
    if (b.index >= frame.size()) {
        throw std::runtime_error("Frame index too large");
    }
    auto bone_node = make_dunique<SceneNode>();
    bone_node->set_position(frame.at(b.index).offset().casted<double>());
    bone_node->set_rotation(frame.at(b.index).quaternion().to_tait_bryan_angles());
    scene_node_resources.instantiate_renderable(
        "frame_bone",
        InstantiationOptions{
            .instance_name = "frame_bone",
            .scene_node = bone_node.ref(DP_LOC),
            .renderable_resource_filter = RenderableResourceFilter{}});
    DanglingRef<SceneNode> parent = bone_node.ref(DP_LOC);
    parent_node->add_child("frame_bone" + std::to_string(b.index), std::move(bone_node));
    for (const auto& c : b.children) {
        add_bone_frame(*c, frame, parent, scene_node_resources);
    }
}

struct LightAndNode {
    Light& light;
    DanglingRef<SceneNode> node;
};

template <class TPos>
LoadMeshConfig<TPos> cfg(const ParsedArgs& args, const std::string& light_configuration) {
    std::vector<BlendMapTexture> textures;
    float period_world = 0.f;
    if (args.has_named_value("--multilayer_diffuse")) {
        textures = {BlendMapTexture{
            .texture_descriptor = {
                .color = args.named_value("--multilayer_diffuse"),
                .normal = args.named_value("--multilayer_normal", ""),
                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
            .role = BlendMapRole::DETAIL_BASE}};
        std::vector<float> lcm_world_args;
        for (uint32_t i = 0; i < 4; ++i) {
            auto detail = args.named_value("--multilayer_detail" + std::to_string(i), "");
            if (detail.empty()) {
                continue;
            }
            float multilayer_mult = safe_stof(args.named_value("--multilayer_mult" + std::to_string(i)));
            if (args.has_named_value("--multilayer_mask")) {
                textures.push_back(BlendMapTexture{
                    .texture_descriptor = {
                        .color = args.named_value("--multilayer_mask"),
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                    .role = BlendMapRole::DETAIL_MASK_R + i});
            }
            textures.push_back(BlendMapTexture{
                .texture_descriptor = {
                    .color = detail,
                    .normal = args.named_value("--multilayer_detail_normal" + std::to_string(i), ""),
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                .scale = multilayer_mult,
                .role = BlendMapRole::DETAIL_COLOR_HORIZONTAL});
            lcm_world_args.push_back(multilayer_mult);
        }
        period_world = least_common_multiple(lcm_world_args.begin(), lcm_world_args.end(), 1e-6f, 1000);
    }
    return LoadMeshConfig<TPos>{
        .position = fixed_zeros<TPos, 3>(),
        .rotation = fixed_zeros<float, 3>(),
        .scale = FixedArray<float, 3>{
            safe_stof(args.named_value("--scale_x", "1")),
            safe_stof(args.named_value("--scale_y", "1")),
            safe_stof(args.named_value("--scale_z", "1"))} * safe_stof(args.named_value("--scale", "1")),
        .blend_mode = blend_mode_from_string(args.named_value("--blend_mode", "binary_05")),
        .cull_faces_default = !args.has_named("--no_cull_faces_default"),
        .cull_faces_alpha = args.has_named("--cull_faces_alpha"),
        .occluded_pass = args.has_named("--no_shadows") || (light_configuration == "none") || (light_configuration == "emissive")
            ? ExternalRenderPassType::NONE
            : ExternalRenderPassType::LIGHTMAP_DEPTH,
        .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
        .magnifying_interpolation_mode = interpolation_mode_from_string(args.named_value("--magnifying_interpolation_mode", "nearest")),
        .aggregate_mode = aggregate_mode_from_string(args.named_value("--aggregate_mode", "none")),
        .transformation_mode = TransformationMode::ALL,
        .textures = textures,
        .period_world = period_world,
        .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
        .apply_static_lighting = args.has_named("--apply_static_lighting"),
        .laplace_ao_strength = safe_stof(args.named_value("--laplace_ao_strength", "0")),
        .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
        .werror = !args.has_named("--no_werror")};
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: render_obj_file\n"
        "    <filename ...>\n"
        "    [--bvh <filename>]\n"
        "    [--animation_frame <id>]\n"
        "    [--bone_frame <id>]\n"
        "    [--hide_object]\n"
        "    [--scale <scale>]\n"
        "    [--scale_x <scale>]\n"
        "    [--scale_y <scale>]\n"
        "    [--scale_z <scale>]\n"
        "    [--node_scale <scale>]\n"
        "    [--bvh_scale <scale>]\n"
        "    [--bvh_demean]\n"
        "    [--bvh_rotation_0]\n"
        "    [--bvh_rotation_1]\n"
        "    [--bvh_rotation_2]\n"
        "    [--bvh_trafo]\n"
        "    [--loop_begin]\n"
        "    [--loop_end]\n"
        "    [--loop_time]\n"
        "    [--speed]\n"
        "    [--reference_bone <filename>]\n"
        "    [--frame_bone <filename>]\n"
        "    [--bone_scale <scale>]\n"
        "    [--y_fov <value>]\n"
        "    [--x <x>]\n"
        "    [--y <y>]\n"
        "    [--z <z>]\n"
        "    [--angle_x <angle_x>]\n"
        "    [--angle_y <angle_y>]\n"
        "    [--angle_z <angle_z>]\n"
        "    [--light_x <x>]\n"
        "    [--light_y <y>]\n"
        "    [--light_z <z>]\n"
        "    [--light_angle_x <angle_x>]\n"
        "    [--light_angle_y <angle_y>]\n"
        "    [--light_angle_z <angle_z>]\n"
        "    [--camera_x <x>]\n"
        "    [--camera_y <y>]\n"
        "    [--camera_z <z>]\n"
        "    [--camera_angle_x <angle_x>]\n"
        "    [--camera_angle_y <angle_y>]\n"
        "    [--camera_angle_z <angle_z>]\n"
        "    [--nsamples_msaa <nsamples>]\n"
        "    [--lightmap_nsamples_msaa <nsamples>]\n"
        "    [--blend_mode {off,continuous,binary,binary_add}]\n"
        "    [--aggregate_mode {off, once, sorted}]\n"
        "    [--cull_faces_render]\n"
        "    [--no_cull_faces_default]\n"
        "    [--cull_faces_alpha]\n"
        "    [--wire_frame]\n"
        "    [--render_dt <dt>]\n"
        "    [--print_render_fps]\n"
        "    [--width <width>]\n"
        "    [--height <height>]\n"
        "    [--output_width <width>]\n"
        "    [--output_height <height>]\n"
        "    [--no_normalmaps]\n"
        "    [--double_buffer]\n"
        "    [--large_object_mode]\n"
        "    [--output <file.png>]\n"
        "    [--output_pass <pass>]\n"
        "    [--output_light_node <node>]\n"
        "    [--apply_static_lighting]\n"
        "    [--laplace_ao_strength <value>]\n"
        "    [--min_num] <min_num>\n"
        "    [--include] <regex>\n"
        "    [--exclude] <regex>\n"
        "    [--no_werror]\n"
        "    [--color_gradient_min_x] <value>\n"
        "    [--color_gradient_max_x] <value>\n"
        "    [--color_gradient_min_c] <value>\n"
        "    [--color_gradient_max_c] <value>\n"
        "    [--color_radial_center_x] <value>\n"
        "    [--color_radial_center_y] <value>\n"
        "    [--color_radial_center_z] <value>\n"
        "    [--color_radial_min_r] <value>\n"
        "    [--color_radial_max_r] <value>\n"
        "    [--color_radial_min_c] <value>\n"
        "    [--color_radial_max_c] <value>\n"
        "    [--color_cone_x] <value>\n"
        "    [--color_cone_z] <value>\n"
        "    [--color_cone_bottom] <value>\n"
        "    [--color_cone_top] <value>\n"
        "    [--color_cone_min_r] <value>\n"
        "    [--color_cone_max_r] <value>\n"
        "    [--color_cone_min_c] <value>\n"
        "    [--color_cone_max_c] <value>\n"
        "    [--color_r] <value>\n"
        "    [--color_g] <value>\n"
        "    [--color_b] <value>\n"
        "    [--background_r] <value>\n"
        "    [--background_g] <value>\n"
        "    [--background_b] <value>\n"
        "    [--background_light_ambience <background_light_ambience>]\n"
        "    [--ambience] <value>\n"
        "    [--diffusivity] <value>\n"
        "    [--specularity] <value>\n"
        "    [--no_shadows]\n"
        "    [--light_configuration {none, emissive, one, shifted_circle, circle}]\n"
        "    [--triangle_tangent_error_behavior {zero, warn, raise}]\n"
        "    [--light_beacon] <filename>\n"
        "    [--light_beacon_scale] <scale>\n"
        "    [--look_at_aabb]\n"
        "    [--multilayer_diffuse <value>]\n"
        "    [--multilayer_normal <value>]\n"
        "    [--multilayer_mask <value>]\n"
        "    [--multilayer_detail0-3 <value>]\n"
        "    [--multilayer_detail_normal0-3 <value>]\n"
        "    [--multilayer_mult0-3 <value>]\n"
        "    [--magnifying_interpolation_mode <value>]\n"
        "Keys: Left, Right, Up, Down, PgUp, PgDown, Ctrl as modifier",
        {"--hide_object",
         "--cull_faces_render",
         "--no_cull_faces_default",
         "--cull_faces_alpha",
         "--wire_frame",
         "--double_buffer",
         "--large_object_mode",
         "--no_normalmaps",
         "--print_render_fps",
         "--no_werror",
         "--apply_static_lighting",
         "--no_shadows",
         "--bvh_demean",
         "--look_at_aabb"},
        {"--bvh",
         "--bvh_rotation_0",
         "--bvh_rotation_1",
         "--bvh_rotation_2",
         "--bvh_trafo",
         "--loop_begin",
         "--loop_end",
         "--loop_time",
         "--speed",
         "--reference_bone",
         "--frame_bone",
         "--bone_scale",
         "--animation_frame",
         "--bone_frame",
         "--scale",
         "--scale_x",
         "--scale_y",
         "--scale_z",
         "--node_scale",
         "--bvh_scale",
         "--y_fov",
         "--x",
         "--y",
         "--z",
         "--angle_x",
         "--angle_y",
         "--angle_z",
         "--light_x",
         "--light_y",
         "--light_z",
         "--light_angle_x",
         "--light_angle_y",
         "--light_angle_z",
         "--camera_x",
         "--camera_y",
         "--camera_z",
         "--camera_angle_x",
         "--camera_angle_y",
         "--camera_angle_z",
         "--nsamples_msaa",
         "--lightmap_nsamples_msaa",
         "--blend_mode",
         "--aggregate_mode",
         "--render_dt",
         "--width",
         "--height",
         "--output_width",
         "--output_height",
         "--output",
         "--output_pass",
         "--output_light_node",
         "--min_num",
         "--include",
         "--exclude",
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
         "--background_b",
         "--ambience",
         "--diffusivity",
         "--specularity",
         "--triangle_tangent_error_behavior",
         "--light_beacon",
         "--light_beacon_scale",
         "--laplace_ao_strength",
         "--multilayer_diffuse",
         "--multilayer_normal",
         "--multilayer_mask",
         "--multilayer_detail0",
         "--multilayer_detail1",
         "--multilayer_detail2",
         "--multilayer_detail3",
         "--multilayer_detail_normal0",
         "--multilayer_detail_normal1",
         "--multilayer_detail_normal2",
         "--multilayer_detail_normal3",
         "--multilayer_mult0",
         "--multilayer_mult1",
         "--multilayer_mult2",
         "--multilayer_mult3",
         "--magnifying_interpolation_mode"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed_atleast(1);

        // Declared as first class to let destructors of other classes succeed.
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderResults render_results;
        RenderedSceneDescriptor rsd{
            .external_render_pass = ExternalRenderPass{
                .pass = args.has_named_value("--output_pass")
                    ? external_render_pass_type_from_string(args.named_value("--output_pass"))
                    : ExternalRenderPassType::STANDARD},
            .light_resource_suffix = args.has_named_value("--output_light_node")
                ? args.named_value("--output_light_node")
                : ""};
        if (args.has_named_value("--output")) {
            render_results.outputs[rsd] = RenderResult{
                .width = safe_stoi(args.named_value("--output_width", "512")),
                .height = safe_stoi(args.named_value("--output_height", "512"))};
        }
        RenderConfig render_config{
            .nsamples_msaa = safe_stoi(args.named_value("--nsamples_msaa", "4")),
            .lightmap_nsamples_msaa = safe_stoi(args.named_value("--lightmap_nsamples_msaa", "1")),
            .cull_faces = args.has_named("--cull_faces_render")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .wire_frame = args.has_named("--wire_frame")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .windowed_width = safe_stoi(args.named_value("--width", "640")),
            .windowed_height = safe_stoi(args.named_value("--height", "480")),
            .double_buffer = args.has_named("--double_buffer"),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = true,
            .print_fps = args.has_named("--print_render_fps"),
            .min_dt = safe_stof(args.named_value("--render_dt", "0.01667"))};
        Render2 render2{
            render_config,
            num_renderings,
            &render_results};

        render2.print_hardware_info();

        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        auto rrg = RenderingContextGuard::root(
            scene_node_resources,
            particle_resources,
            "primary_rendering_resources",
            render_config.anisotropic_filtering_level,
            0);
        AggregateRendererGuard aggregate_renderer_guard{
            std::make_shared<AggregateArrayRenderer>(),
            std::make_shared<AggregateArrayRenderer>()};
        InstancesRendererGuard instances_renderer_guard{
            std::make_shared<ArrayInstancesRenderers>(),
            std::make_shared<ArrayInstancesRenderer>()};
        DeleteNodeMutex delete_node_mutex;
        Scene scene{ delete_node_mutex };
        DestructionGuard scene_destruction_guard{[&](){
            std::scoped_lock lock{ delete_node_mutex };
            scene.shutdown();
        }};
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = make_dunique<SceneNode>();
        // Setting style before adding renderables to avoid race condition.
        if (light_configuration == "emissive") {
            scene_node->add_color_style(std::unique_ptr<ColorStyle>(new ColorStyle{
                .selector = Mlib::compile_regex(""),
                .emissivity = {1.f, 1.f, 1.f}}));
        }
        auto create_light = [&args](const std::string& resource_suffix) {
            if (args.has_named("--no_shadows")) {
                return std::unique_ptr<Light>(new Light{
                    .ambience = fixed_full<float, 3>(safe_stof(args.named_value("--ambience", "1"))),
                    .diffusivity = fixed_full<float, 3>(safe_stof(args.named_value("--diffusivity", "1"))),
                    .specularity = fixed_full<float, 3>(safe_stof(args.named_value("--specularity", "1")))});
            } else {
                return std::unique_ptr<Light>(new Light{
                    .resource_suffix = resource_suffix,
                    .shadow_render_pass = ExternalRenderPassType::LIGHTMAP_DEPTH});
            }
        };
        {
            auto filenames = args.unnamed_values();
            std::vector<std::string> resource_names;
            resource_names.reserve(filenames.size());
            size_t i = 0;
            for (const std::string& filename : filenames) {
                const auto& name = resource_names.emplace_back("obj-" + std::to_string(i++));
                if (filename.ends_with(".obj")) {
                    if (!args.has_named("--large_object_mode")) {
                        scene_node_resources.add_resource(name, load_renderable_obj(
                            filename,
                            cfg<float>(args, light_configuration),
                            scene_node_resources));
                    } else {
                        scene_node_resources.add_resource(name, load_renderable_obj(
                            filename,
                            cfg<double>(args, light_configuration),
                            scene_node_resources));
                    }
                } else if (filename.ends_with(".kn5") || filename.ends_with(".ini")) {
                    if (!args.has_named("--large_object_mode")) {
                        scene_node_resources.add_resource(name, load_renderable_kn5(
                            filename,
                            cfg<float>(args, light_configuration),
                            scene_node_resources,
                            RenderingContextStack::primary_rendering_resources().get(),
                            nullptr)); // race_logic
                    } else {
                        scene_node_resources.add_resource(name, load_renderable_kn5(
                            filename,
                            cfg<double>(args, light_configuration),
                            scene_node_resources,
                            RenderingContextStack::primary_rendering_resources().get(),
                            nullptr)); // race_logic
                    }
                } else if (filename.ends_with(".mhx2")) {
                    auto rmhx2 = std::make_shared<Mhx2FileResource>(
                        filename,
                        cfg<float>(args, light_configuration));
                    scene_node_resources.add_resource(name, rmhx2);
                    scene_node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
                        .periodic_skelletal_animation_name = "anim",
                        .periodic_skelletal_animation_frame = {
                            .frame = AnimationFrame {
                                .begin = safe_stof(args.named_value("--loop_begin", "0")),
                                .end = safe_stof(args.named_value("--loop_end", "2")),
                                .time = safe_stof(args.named_value("--loop_time", "1"))}}}));
                    LoadMeshConfig<float> bone_cfg{
                        .position = fixed_zeros<float, 3>(),
                        .rotation = fixed_zeros<float, 3>(),
                        .scale = fixed_full<float, 3>(safe_stof(args.named_value("--bone_scale", "1"))),
                        .blend_mode = BlendMode::OFF,
                        .cull_faces_default = !args.has_named("--no_cull_faces_default"),
                        .cull_faces_alpha = args.has_named("--cull_faces_alpha"),
                        .occluded_pass = ExternalRenderPassType::NONE,
                        .occluder_pass = ExternalRenderPassType::NONE,
                        .aggregate_mode = AggregateMode::NONE,
                        .transformation_mode = TransformationMode::ALL,
                        .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
                        .apply_static_lighting = false,
                        .laplace_ao_strength = 0.f,
                        .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
                        .werror = !args.has_named("--no_werror")};
                    if (args.has_named_value("--reference_bone")) {
                        scene_node_resources.add_resource("reference_bone", load_renderable_obj(
                            args.named_value("--reference_bone"),
                            bone_cfg,
                            scene_node_resources));
                        add_reference_bone(rmhx2->skeleton(), scene_node.ref(DP_LOC), scene_node_resources);
                    }
                    if (args.has_named_value("--bvh")) {
                        BvhConfig bvh_config{
                            .demean = args.has_named_value("--bvh_demean") ? safe_stob(args.named_value("--bvh_demean")) : blender_bvh_config.demean,
                            .scale = args.has_named_value("--bvh_scale") ? safe_stof(args.named_value("--bvh_scale")) : blender_bvh_config.scale,
                            .rotation_order = FixedArray<size_t, 3>{
                                args.has_named_value("--bvh_rotation_0") ? safe_stoz(args.named_value("--bvh_rotation_0")) : blender_bvh_config.rotation_order(0),
                                args.has_named_value("--bvh_rotation_1") ? safe_stoz(args.named_value("--bvh_rotation_1")) : blender_bvh_config.rotation_order(1),
                                args.has_named_value("--bvh_rotation_2") ? safe_stoz(args.named_value("--bvh_rotation_2")) : blender_bvh_config.rotation_order(2)},
                            .parameter_transformation = args.has_named_value("--bvh_trafo")
                                ? get_parameter_transformation(args.named_value("--bvh_trafo"))
                                : blender_bvh_config.parameter_transformation};
                        scene_node_resources.add_resource("anim", std::make_shared<BvhFileResource>(args.named_value("--bvh"), bvh_config));
                        if (args.has_named_value("--frame_bone")) {
                            float bone_frame = safe_stof(args.named_value("--bone_frame"));
                            scene_node_resources.add_resource("frame_bone", load_renderable_obj(
                                args.named_value("--frame_bone"),
                                bone_cfg,
                                scene_node_resources));
                            add_bone_frame(
                                rmhx2->skeleton(),
                                rmhx2->vectorize_joint_poses(scene_node_resources.get_relative_poses("anim", bone_frame)),
                                scene_node.ref(DP_LOC),
                                scene_node_resources);
                        }
                        // This invalidates the bone weights and clears the skeleton => must be after "add_bone_frame"
                        if (args.has_named_value("--animation_frame")) {
                            float animation_frame = safe_stof(args.named_value("--animation_frame"));
                            scene_node_resources.set_relative_joint_poses(name, scene_node_resources.get_relative_poses("anim", animation_frame));
                        }
                    }
                } else {
                    throw std::runtime_error("File has unknown extension: " + filename);
                }
            }
            scene_node_resources.add_resource("objs", std::make_shared<CompoundResource>(scene_node_resources, resource_names));
            merge_textures(
                "objs",
                MergedTexturesConfig{
                    .resource_name = "merged_resource",
                    .array_name = "merged_array",
                    .texture_name = "merged_texture",
                    .blend_mode = BlendMode::SEMI_CONTINUOUS_02,
                    .continuous_blending_z_order = 1,
                    .aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY,
                    .max_triangle_distance = INFINITY,
                    .cull_faces = false,
                    .ambience = {1.f, 1.f, 1.f}
                },
                scene_node_resources,
                *RenderingContextStack::primary_rendering_resources());
            {
                scene_node->set_position({
                    safe_stof(args.named_value("--x", "0")),
                    safe_stof(args.named_value("--y", "0")),
                    safe_stof(args.named_value("--z", "-40"))});
                scene_node->set_rotation({
                    safe_stof(args.named_value("--angle_x", "0")) * degrees,
                    safe_stof(args.named_value("--angle_y", "0")) * degrees,
                    safe_stof(args.named_value("--angle_z", "0")) * degrees});
                scene_node->set_scale(safe_stof(args.named_value("--node_scale", "1")));
                if (!args.has_named("--hide_object")) {
                    scene_node_resources.instantiate_renderable(
                        "objs",
                        InstantiationOptions{
                            .instance_name = "objs",
                            .scene_node = scene_node.ref(DP_LOC),
                            .renderable_resource_filter = RenderableResourceFilter{
                                .min_num = safe_stoz(args.named_value("--min_num", "0")),
                                .cva_filter = {
                                    .included_names = Mlib::compile_regex(args.named_value("--include", "")),
                                    .excluded_names = Mlib::compile_regex(args.named_value("--exclude", "$ ^"))}}},
                        PreloadBehavior::NO_PRELOAD);
                }
                if (args.has_named_value("--color_gradient_min_x") || args.has_named_value("--color_gradient_max_x")) {
                    auto apply_color_gradient = [&args]<typename TPos>(std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
                    {
                        Interp<TPos> interp{
                            {safe_sto<TPos>(args.named_value("--color_gradient_min_x")),
                            safe_sto<TPos>(args.named_value("--color_gradient_max_x"))},
                            {safe_sto<float>(args.named_value("--color_gradient_min_c")),
                            safe_sto<float>(args.named_value("--color_gradient_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = (float)interp(v.position(0));
                                }
                            }
                        }
                    };
                    for (const auto& acva : scene_node_resources.get_rendering_arrays("objs")) {
                        apply_color_gradient(acva->scvas);
                        apply_color_gradient(acva->dcvas);
                    }
                }
                if (args.has_named_value("--color_radial_min_r") || args.has_named_value("--color_radial_max_r")) {
                    auto apply_radial_colors = [&args]<typename TPos>(std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
                    {
                        Interp<TPos> interp{
                            {safe_sto<TPos>(args.named_value("--color_radial_min_r")),
                            safe_sto<TPos>(args.named_value("--color_radial_max_r"))},
                            {safe_sto<float>(args.named_value("--color_radial_min_c")),
                            safe_sto<float>(args.named_value("--color_radial_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        FixedArray<TPos, 3> center{
                            safe_sto<TPos>(args.named_value("--color_radial_center_x", "0")),
                            safe_sto<TPos>(args.named_value("--color_radial_center_y", "0")),
                            safe_sto<TPos>(args.named_value("--color_radial_center_z", "0"))};
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = (float)interp(std::sqrt(sum(squared(v.position - center))));
                                }
                            }
                        }
                    };
                    for (const auto& acva : scene_node_resources.get_rendering_arrays("objs")) {
                        apply_radial_colors(acva->scvas);
                        apply_radial_colors(acva->dcvas);
                    }
                }
                if (args.has_named_value("--color_cone_min_r") || args.has_named_value("--color_cone_max_r")) {
                    auto apply_cone_colors = [&args]<typename TPos>(std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas) {
                        Interp<TPos> interp{
                            {safe_sto<TPos>(args.named_value("--color_cone_min_r")),
                            safe_sto<TPos>(args.named_value("--color_cone_max_r"))},
                            {safe_sto<float>(args.named_value("--color_cone_min_c")),
                            safe_sto<float>(args.named_value("--color_cone_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        TPos bottom = safe_sto<float>(args.named_value("--color_cone_bottom", "0"));
                        TPos top = safe_sto<float>(args.named_value("--color_cone_top"));
                        TPos cx = safe_sto<float>(args.named_value("--color_cone_x", "0"));
                        TPos cz = safe_sto<float>(args.named_value("--color_cone_z", "0"));
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    TPos r = std::sqrt(squared(v.position(0) - cx) + squared(v.position(2) - cz));
                                    TPos h = (top - v.position(1)) / (top - bottom);
                                    v.color = (float)interp(r / h);
                                }
                            }
                        }
                    };
                    for (const auto& acva : scene_node_resources.get_rendering_arrays("objs")) {
                        apply_cone_colors(acva->scvas);
                        apply_cone_colors(acva->dcvas);
                    }
                }
                auto apply_constant_color = [&args]<typename TPos>(std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas) {
                    FixedArray<TPos, 3> color{
                        safe_sto<TPos>(args.named_value("--color_r", "-1")),
                        safe_sto<TPos>(args.named_value("--color_g", "-1")),
                        safe_sto<TPos>(args.named_value("--color_b", "-1"))};
                    if (any(color != TPos(-1))) {
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = maximum(color TEMPLATEV casted<float>(), 0.f);
                                }
                            }
                        }
                    }
                };
                for (const auto& acva : scene_node_resources.get_rendering_arrays("objs")) {
                    apply_constant_color(acva->scvas);
                    apply_constant_color(acva->dcvas);
                }
            }
        }
        if (args.has_named("--large_object_mode")) {
            scene.add_root_aggregate_node("obj", std::move(scene_node));
        } else {
            scene.add_root_node("obj", std::move(scene_node));
        }

        size_t light_beacon_index = 0;
        auto add_light_beacon_if_set = [&args, &light_beacon_index, &scene_node_resources](DanglingRef<SceneNode> scene_node){
            if (!args.has_named_value("--light_beacon")) {
                return;
            }
            std::string name = "light_beacon-" + std::to_string(light_beacon_index++);
            LoadMeshConfig<float> cfg{
                .position = fixed_zeros<float, 3>(),
                .rotation = fixed_zeros<float, 3>(),
                .scale = fixed_full<float, 3>(safe_stof(args.named_value("--light_beacon_scale", "1"))),
                .blend_mode = blend_mode_from_string(args.named_value("--blend_mode", "binary_05")),
                .cull_faces_default = !args.has_named("--no_cull_faces_default"),
                .cull_faces_alpha = args.has_named("--cull_faces_alpha"),
                .occluded_pass = ExternalRenderPassType::NONE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .aggregate_mode = AggregateMode::NONE,
                .transformation_mode = TransformationMode::ALL,
                .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
                .apply_static_lighting = args.has_named("--apply_static_lighting"),
                .laplace_ao_strength = 0.f,
                .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
                .werror = !args.has_named("--no_werror")};
            scene_node_resources.add_resource(name, load_renderable_obj(
                args.named_value("--light_beacon"),
                cfg,
                scene_node_resources));
            scene_node_resources.instantiate_renderable(
                name,
                InstantiationOptions{
                    .instance_name = name,
                    .scene_node = scene_node,
                    .renderable_resource_filter = RenderableResourceFilter{}});
        };
        std::list<LightAndNode> lights;
        SelectedCameras selected_cameras{scene};
        if (light_configuration == "one") {
            scene.add_root_node("light_node0", make_dunique<SceneNode>());
            scene.get_node("light_node0", DP_LOC)->set_position({
                safe_stof(args.named_value("--light_x", "0")),
                safe_stof(args.named_value("--light_y", "50")),
                safe_stof(args.named_value("--light_z", "0"))});
            scene.get_node("light_node0", DP_LOC)->set_rotation({
                safe_stof(args.named_value("--light_angle_x", "-45")) * degrees,
                safe_stof(args.named_value("--light_angle_y", "0")) * degrees,
                safe_stof(args.named_value("--light_angle_z", "0")) * degrees});
            auto light = create_light("light_node0");
            lights.push_back({.light = *light, .node = scene.get_node("light_node0", DP_LOC)});
            scene.get_node("light_node0", DP_LOC)->add_light(std::move(light));
            scene.get_node("light_node0", DP_LOC)->set_camera(
                std::make_unique<PerspectiveCamera>(
                    PerspectiveCameraConfig(),
                    PerspectiveCamera::Postprocessing::ENABLED));
            add_light_beacon_if_set(scene.get_node("light_node0", DP_LOC));
        } else if (light_configuration == "circle" || light_configuration == "shifted_circle") {
            size_t n = 10;
            float r = 50;
            size_t i = 0;
            bool with_diffusivity = true;
            FixedArray<float, 3> center;
            if (light_configuration == "circle") {
                center = {0.f, 10.f, 0.f};
            } else if (light_configuration == "shifted_circle") {
                center = {-50.f, 50.f, -20.f};
            } else {
                throw std::runtime_error("Unknown light configuration");
            }
            for (float a : Linspace<float>(0.f, 2.f * float(M_PI), n)) {
                std::string name = "light" + std::to_string(i++);
                scene.add_root_node(name, make_dunique<SceneNode>());
                scene.get_node(name, DP_LOC)->set_position({float(r * cos(a)) + center(0), center(1), float(r * sin(a)) + center(2)});
                scene.get_node(name, DP_LOC)->set_rotation(matrix_2_tait_bryan_angles(gl_lookat_absolute(
                    scene.get_node(name, DP_LOC)->position(),
                    scene.get_node("obj", DP_LOC)->position())).casted<float>());
                auto light = create_light(name);
                lights.push_back({.light = *light, .node = scene.get_node(name, DP_LOC)});
                scene.get_node(name, DP_LOC)->add_light(std::move(light));
                scene.get_node(name, DP_LOC)->set_camera(
                    std::make_unique<PerspectiveCamera>(
                        PerspectiveCameraConfig(),
                        PerspectiveCamera::Postprocessing::ENABLED));
                lights.back().light.ambience *= 2.f / float(n * size_t(1 + (int)with_diffusivity));
                lights.back().light.diffusivity = 0.f;
                lights.back().light.specularity = 0.f;
            }
            if (with_diffusivity) {
                for (float a : Linspace<float>(0.f, 2.f * float(M_PI), n)) {
                    std::string name = "light_s" + std::to_string(i++);
                    scene.add_root_node(name, make_dunique<SceneNode>());
                    scene.get_node(name, DP_LOC)->set_position({float(r * cos(a)) + center(0), center(1), float(r * sin(a)) + center(2)});
                    scene.get_node(name, DP_LOC)->set_rotation(matrix_2_tait_bryan_angles(gl_lookat_absolute(
                        scene.get_node(name, DP_LOC)->position(),
                        scene.get_node("obj", DP_LOC)->position())).casted<float>());
                    auto light = create_light(name);
                    lights.push_back({.light = *light, .node = scene.get_node(name, DP_LOC)});
                    scene.get_node(name, DP_LOC)->add_light(std::move(light));
                    scene.get_node(name, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                        PerspectiveCameraConfig(),
                        PerspectiveCamera::Postprocessing::ENABLED));
                    lights.back().light.ambience = 0.f;
                    lights.back().light.diffusivity /= (float)(2 * n);
                    lights.back().light.specularity = 0.f;
                }
            }
        } else if ((light_configuration != "none") && (light_configuration != "emissive")) {
            throw std::runtime_error("Unknown light configuration");
        }
        if (args.has_named_value("--background_light_ambience")) {
            std::string name = "background_light";
            scene.add_root_node(name, make_dunique<SceneNode>());
            auto light = create_light(name);
            lights.push_back({.light = *light, .node = scene.get_node(name, DP_LOC)});
            scene.get_node(name, DP_LOC)->add_light(std::move(light));
            scene.get_node(name, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                PerspectiveCameraConfig(),
                PerspectiveCamera::Postprocessing::ENABLED));
            lights.back().light.ambience = FixedArray<float, 3>{1.f, 1.f, 1.f} * safe_stof(args.named_value("--background_light_ambience"));
            lights.back().light.diffusivity = 0.f;
            lights.back().light.specularity = 0.f;
        }
        
        scene.add_root_node("follower_camera", make_dunique<SceneNode>());
        if (args.has_named("--look_at_aabb")) {
            auto aabb = scene.get_node("obj", DP_LOC)->relative_aabb();
            if (!aabb.has_value()) {
                throw std::runtime_error("Node has no AABB");
            }
            auto la = gl_lookat_aabb(
                scene.get_node("follower_camera", DP_LOC)->position(),
                scene.get_node("obj", DP_LOC)->absolute_model_matrix(),
                aabb.value());
            if (!la.has_value()) {
                throw std::runtime_error("Could not compute frustum");
            }
            auto npixels = npixels_for_dpi(
                la.value().sensor_aabb,
                PerspectiveCameraConfig().dpi((float)render_config.windowed_height),
                1,
                2048);
            if (!npixels.has_value()) {
                throw std::runtime_error("Could not compute npixels, object might be too small or too large");
            }
            if (args.has_named_value("--output")) {
                render_results.outputs.at(rsd).width = npixels.value().width;
                render_results.outputs.at(rsd).height = npixels.value().height;
            }
            scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<FrustumCamera>(
                FrustumCameraConfig::from_sensor_aabb(
                    npixels.value().scaled_sensor_aabb,
                    la.value().near_plane,
                    la.value().far_plane),
                FrustumCamera::Postprocessing::ENABLED));
            scene.get_node("follower_camera", DP_LOC)->set_rotation(matrix_2_tait_bryan_angles(la.value().extrinsic_R));
        } else {
            scene.get_node("follower_camera", DP_LOC)->set_position({
                safe_stof(args.named_value("--camera_x", "0")),
                safe_stof(args.named_value("--camera_y", "0")),
                safe_stof(args.named_value("--camera_z", "0"))});
            scene.get_node("follower_camera", DP_LOC)->set_rotation({
                safe_stof(args.named_value("--camera_angle_x", "0")) * degrees,
                safe_stof(args.named_value("--camera_angle_y", "0")) * degrees,
                safe_stof(args.named_value("--camera_angle_z", "0")) * degrees});
            scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                PerspectiveCameraConfig{
                    .y_fov = safe_stof(args.named_value("--y_fov", "90")) * degrees},
                PerspectiveCamera::Postprocessing::ENABLED));
        }
        
        // scene.print();
        Focuses focuses = {Focus::SCENE};
        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        StandardCameraLogic standard_camera_logic{
            scene,
            selected_cameras,
            delete_node_mutex};
        StandardRenderLogic standard_render_logic{
            scene,
            standard_camera_logic,
            {
                safe_stof(args.named_value("--background_r", "1")),
                safe_stof(args.named_value("--background_g", "0")),
                safe_stof(args.named_value("--background_b", "1"))},
            ClearMode::COLOR_AND_DEPTH};
        FlyingCameraUserClass user_object{
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .cameras = selected_cameras,
            .focuses = focuses,
            .wire_frame = render_config.wire_frame,
            .depth_test = render_config.depth_test,
            .cull_faces = render_config.cull_faces,
            .delete_node_mutex = delete_node_mutex,
            .physics_set_fps = nullptr};
        auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
            render2.glfw_window(),
            button_states,
            scene,
            user_object,
            true,                                       // fly
            !args.has_named("--large_object_mode"));    // rotate
        auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);
        std::list<std::shared_ptr<LightmapLogic>> lightmap_logics;
        for (const auto& l : lights) {
            if (any(l.light.shadow_render_pass & ExternalRenderPassType::LIGHTMAP_DEPTH)) {
                lightmap_logics.push_back(std::make_shared<LightmapLogic>(
                    *read_pixels_logic,
                    l.light.shadow_render_pass,
                    l.node,
                    l.light.resource_suffix,
                    "",                             // black_node_name
                    true,                           // with_depth_texture
                    2048,                           // lightmap_width
                    2048));                         // lightmap_height
            }
        }

        UiFocus ui_focus;
        RenderLogics render_logics{ui_focus};
        render_logics.append(nullptr, flying_camera_logic);
        for (const auto& l : lightmap_logics) {
            render_logics.append(nullptr, l);
        }
        render_logics.append(nullptr, read_pixels_logic);
        // The following is required for animations.
        render_logics.append(nullptr, std::make_shared<MoveSceneLogic>(
            scene,
            delete_node_mutex,
            safe_stof(args.named_value("--speed", "1"))));
        LambdaRenderLogic lrl{
            [&delete_node_mutex, &render_logics](
                const LayoutConstraintParameters& lx,
                const LayoutConstraintParameters& ly,
                const RenderConfig& render_config,
                const SceneGraphConfig& scene_graph_config,
                RenderResults* render_results,
                const RenderedSceneDescriptor& frame_id)
            {
                std::scoped_lock lock{delete_node_mutex};
                render_logics.render(lx, ly, render_config, scene_graph_config, render_results, frame_id);
            }
        };
        render2.render(
            lrl,
            SceneGraphConfig(),
            &button_states);
        if (unhandled_exceptions_occured()) {
            print_unhandled_exceptions();
            return 1;
        }
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
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        return 1;
    }
    return 0;
}
