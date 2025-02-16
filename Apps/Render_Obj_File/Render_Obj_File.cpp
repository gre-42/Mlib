#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At_Aabb.hpp>
#include <Mlib/Geometry/Coordinates/Npixels_For_Dpi.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Load/Draw_Distance_Db.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Least_Common_Multiple.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Modifiers/Merge_Textures.hpp>
#include <Mlib/Render/Modifiers/Merged_Textures_Config.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Aggregate_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Menu_Logic.hpp>
#include <Mlib/Render/Render_Logics/Move_Scene_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Window_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Resources/Bvh_File_Resource.hpp>
#include <Mlib/Render/Resources/Dff_File_Resource.hpp>
#include <Mlib/Render/Resources/Kn5_File_Resource.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Modifiers/Add_Cleanup_Mesh_Modifier.hpp>
#include <Mlib/Scene_Graph/Resources/Compound_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Time/Fps/Fixed_Time_Sleeper.hpp>
#include <Mlib/Time/Fps/Measure_Fps.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <Mlib/Time/Fps/Sleeper_Sequence.hpp>
#include <vector>

using namespace Mlib;

/** Add the refernce bone hierarchy to the parent node.
 *
 * The refernce bones have the coordinates of the T-pose, typically.
 * It is accessed by Bone::initial_absolute_transformation.
 */
static void add_reference_bone(
    const Bone& b,
    DanglingRef<SceneNode> parent_node,
    SceneNodeResources& scene_node_resources)
{
    auto bone_node = make_unique_scene_node(
        b.initial_absolute_transformation.t.casted<ScenePos>(),
        b.initial_absolute_transformation.q.to_tait_bryan_angles(),
        1.f,
        PoseInterpolationMode::ENABLED);
    scene_node_resources.instantiate_child_renderable(
        "reference_bone",
        ChildInstantiationOptions{
            .instance_name = VariableAndHash<std::string>{"reference_bone"},
            .scene_node = bone_node.ref(DP_LOC),
            .interpolation_mode = PoseInterpolationMode::ENABLED,
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
static void add_bone_frame(
    const Bone& b,
    const UUVector<OffsetAndQuaternion<float, float>>& frame,
    DanglingRef<SceneNode> parent_node,
    SceneNodeResources& scene_node_resources)
{
    if (b.index >= frame.size()) {
        throw std::runtime_error("Frame index too large");
    }
    auto bone_node = make_unique_scene_node(
        frame.at(b.index).t.casted<ScenePos>(),
        frame.at(b.index).q.to_tait_bryan_angles(),
        1.f,
        PoseInterpolationMode::ENABLED);
    scene_node_resources.instantiate_child_renderable(
        "frame_bone",
        ChildInstantiationOptions{
            .instance_name = VariableAndHash<std::string>{"frame_bone"},
            .scene_node = bone_node.ref(DP_LOC),
            .interpolation_mode = PoseInterpolationMode::ENABLED,
            .renderable_resource_filter = RenderableResourceFilter{}});
    DanglingRef<SceneNode> parent = bone_node.ref(DP_LOC);
    parent_node->add_child("frame_bone" + std::to_string(b.index), std::move(bone_node));
    for (const auto& c : b.children) {
        add_bone_frame(*c, frame, parent, scene_node_resources);
    }
}

struct LightAndNode {
    std::shared_ptr<Light> light;
    DanglingRef<SceneNode> node;
};

template <class TPos>
static LoadMeshConfig<TPos> cfg(const ParsedArgs& args, const std::string& light_configuration) {
    std::vector<BlendMapTexture> textures;
    float period_world = 0.f;
    if (args.has_named_value("--multilayer_diffuse")) {
        textures = {BlendMapTexture{
            .texture_descriptor = {
                .color = {.filename = VariableAndHash{args.named_value("--multilayer_diffuse")}, .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                .normal = {.filename = VariableAndHash{args.named_value("--multilayer_normal", "")}, .color_mode = ColorMode::RGB, .mipmap_mode = MipmapMode::WITH_MIPMAPS}},
            .role = BlendMapRole::DETAIL_BASE,
            .reweight_mode = BlendMapReweightMode::DISABLED}};
        std::vector<float> lcm_world_args;
        for (uint32_t i = 0; i < 4; ++i) {
            auto detail = args.named_value("--multilayer_detail" + std::to_string(i), "");
            if (detail.empty()) {
                continue;
            }
            float multilayer_scale = safe_stof(args.named_value("--multilayer_scale" + std::to_string(i)));
            if (args.has_named_value("--multilayer_mask")) {
                textures.push_back(BlendMapTexture{
                    .texture_descriptor = {
                        .color = {.filename = VariableAndHash{args.named_value("--multilayer_mask")}, .mipmap_mode = MipmapMode::WITH_MIPMAPS}},
                    .role = BlendMapRole::DETAIL_MASK_R + i});
            }
            textures.push_back(BlendMapTexture{
                .texture_descriptor = {
                    .color = {.filename = VariableAndHash{detail}, .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                    .normal = {
                        .filename = VariableAndHash{args.named_value("--multilayer_detail_normal" + std::to_string(i), "")},
                        .color_mode = ColorMode::RGB,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS}},
                .scale = multilayer_scale,
                .role = BlendMapRole::DETAIL_COLOR,
                .uv_source = BlendMapUvSource::VERTICAL0});
            lcm_world_args.push_back(multilayer_scale);
        }
        if (!lcm_world_args.empty()) {
            period_world = least_common_multiple(lcm_world_args.begin(), lcm_world_args.end(), 1e-6f, 10'000);
        }
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
        .magnifying_interpolation_mode = interpolation_mode_from_string(args.named_value("--magnifying_interpolation_mode", "linear")),
        .aggregate_mode = aggregate_mode_from_string(args.named_value("--aggregate_mode", "none")),
        .transformation_mode = TransformationMode::ALL,
        .fresnel = {
            .reflectance = {
                .min = safe_stof(args.named_value("--fresnel_min", "0")),
                .max = safe_stof(args.named_value("--fresnel_max", "0")),
                .exponent = safe_stof(args.named_value("--fresnel_exponent", "0"))
            },
            .ambient = {
                safe_stof(args.named_value("--fresnel_r", "0")),
                safe_stof(args.named_value("--fresnel_g", "0")),
                safe_stof(args.named_value("--fresnel_b", "0"))}
        },
        .textures = textures,
        .period_world = period_world,
        .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
        .apply_static_lighting = args.has_named("--apply_static_lighting"),
        .laplace_ao_strength = safe_stof(args.named_value("--laplace_ao_strength", "0")),
        .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
        .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
        .werror = !args.has_named("--no_werror")};
}

int main(int argc, char** argv) {
    reserve_realtime_threads(0);
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
        "    [--render_sleep_dt <dt>]\n"
        "    [--print_render_fps_interval <n>]\n"
        "    [--width <width>]\n"
        "    [--height <height>]\n"
        "    [--output_width <width>]\n"
        "    [--output_height <height>]\n"
        "    [--no_normalmaps]\n"
        "    [--no_double_buffer]\n"
        "    [--large_object_mode]\n"
        "    [--output <file.png>]\n"
        "    [--output_pass <pass>]\n"
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
        "    [--ambient] <value>\n"
        "    [--diffuse] <value>\n"
        "    [--specular] <value>\n"
        "    [--fresnel_r] <value>\n"
        "    [--fresnel_g] <value>\n"
        "    [--fresnel_b] <value>\n"
        "    [--fresnel_min] <value>\n"
        "    [--fresnel_min] <value>\n"
        "    [--fresnel_exponent] <value>\n"
        "    [--no_shadows]\n"
        "    [--light_configuration {none, emissive, one, shifted_circle, circle}]\n"
        "    [--triangle_tangent_error_behavior {zero, warn, raise}]\n"
        "    [--light_beacon] <filename>\n"
        "    [--light_beacon_scale] <scale>\n"
        "    [--look_at_aabb]\n"
        "    [--cleanup_mesh]\n"
        "    [--multilayer_diffuse <value>]\n"
        "    [--multilayer_normal <value>]\n"
        "    [--multilayer_mask <value>]\n"
        "    [--multilayer_detail0-3 <value>]\n"
        "    [--multilayer_detail_normal0-3 <value>]\n"
        "    [--multilayer_scale0-3 <value>]\n"
        "    [--magnifying_interpolation_mode <value>]\n"
        "Keys: Left, Right, Up, Down, PgUp, PgDown, Ctrl as modifier",
        {"--hide_object",
         "--cull_faces_render",
         "--no_cull_faces_default",
         "--cull_faces_alpha",
         "--wire_frame",
         "--no_double_buffer",
         "--large_object_mode",
         "--no_normalmaps",
         "--no_werror",
         "--apply_static_lighting",
         "--no_shadows",
         "--bvh_demean",
         "--look_at_aabb",
         "--cleanup_mesh"},
        {"--bvh",
         "--bvh_rotation_0",
         "--bvh_rotation_1",
         "--bvh_rotation_2",
         "--bvh_trafo",
         "--print_render_fps_interval",
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
         "--render_sleep_dt",
         "--width",
         "--height",
         "--output_width",
         "--output_height",
         "--output",
         "--output_pass",
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
         "--ambient",
         "--diffuse",
         "--specular",
         "--fresnel_r",
         "--fresnel_g",
         "--fresnel_b",
         "--fresnel_min",
         "--fresnel_max",
         "--fresnel_exponent",
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
         "--multilayer_scale0",
         "--multilayer_scale1",
         "--multilayer_scale2",
         "--multilayer_scale3",
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
                    : ExternalRenderPassType::STANDARD} };
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
            .double_buffer = !args.has_named("--no_double_buffer"),
            .normalmaps = !args.has_named("--no_normalmaps"),
            .show_mouse_cursor = true};
        InputConfig input_config;
        FixedTimeSleeper sleeper{ safe_stof(args.named_value("--sleep_dt", "0.01667")) };
        MeasureFps mf{
            0.005f,
            safe_stou(args.named_value("--print_render_fps_interval", "-1"))};
        SleeperSequence sls{{ &mf, &sleeper }};
        SetFps set_fps{
            &sls,
            std::function<std::chrono::steady_clock::time_point()>(),
            []() { return false; }};
        Render render{
            render_config,
            input_config,
            num_renderings,
            set_fps,
            []() { return std::chrono::steady_clock::now(); },
            &render_results };

        render.print_hardware_info();
        ClearWrapperGuard clear_wrapper_guard;

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
        AggregateRendererGuard aggregate_renderer_guard{
            std::make_shared<AggregateArrayRenderer>(rendering_resources),
            std::make_shared<AggregateArrayRenderer>(rendering_resources)};
        InstancesRendererGuard instances_renderer_guard{
            std::make_shared<ArrayInstancesRenderers>(rendering_resources),
            std::make_shared<ArrayInstancesRenderer>(rendering_resources)};
        DeleteNodeMutex delete_node_mutex;
        Scene scene{ "main_scene", delete_node_mutex };
        DestructionGuard scene_destruction_guard{[&](){
            scene.shutdown();
        }};
        std::string light_configuration = args.named_value("--light_configuration", "one");
        auto scene_node = make_unique_scene_node(
            FixedArray<ScenePos, 3>{
                safe_stox<ScenePos>(args.named_value("--x", "0")),
                safe_stox<ScenePos>(args.named_value("--y", "0")),
                safe_stox<ScenePos>(args.named_value("--z", "-40"))},
            FixedArray<float, 3>{
                safe_stof(args.named_value("--angle_x", "0")) * degrees,
                safe_stof(args.named_value("--angle_y", "0")) * degrees,
                safe_stof(args.named_value("--angle_z", "0")) * degrees},
            safe_stof(args.named_value("--node_scale", "1")));
        // Setting style before adding renderables to avoid race condition.
        if (light_configuration == "emissive") {
            scene_node->add_color_style(std::unique_ptr<ColorStyle>(new ColorStyle{
                .emissive = {1.f, 1.f, 1.f}}));
        }
        auto create_light = [&args](const std::string& resource_suffix) {
            if (args.has_named("--no_shadows")) {
                return std::make_shared<Light>(Light{
                    .ambient = fixed_full<float, 3>(safe_stof(args.named_value("--ambient", "1"))),
                    .diffuse = fixed_full<float, 3>(safe_stof(args.named_value("--diffuse", "1"))),
                    .specular = fixed_full<float, 3>(safe_stof(args.named_value("--specular", "1"))),
                    .shadow_render_pass = ExternalRenderPassType::NONE});
            } else {
                return std::make_shared<Light>(Light{
                    .ambient = fixed_full<float, 3>(safe_stof(args.named_value("--ambient", "1"))),
                    .diffuse = fixed_full<float, 3>(safe_stof(args.named_value("--diffuse", "1"))),
                    .specular = fixed_full<float, 3>(safe_stof(args.named_value("--specular", "1"))),
                    .lightmap_depth = nullptr,
                    .shadow_render_pass = ExternalRenderPassType::LIGHTMAP_DEPTH});
            }
        };
        {
            auto filenames = args.unnamed_values();
            std::vector<std::string> resource_names;
            resource_names.reserve(filenames.size());
            for (const auto& [i, filename] : enumerate(filenames)) {
                const auto& name = resource_names.emplace_back("obj-" + std::to_string(i));
                if (filename.ends_with(".obj")) {
                    if (!args.has_named("--large_object_mode")) {
                        scene_node_resources.add_resource(name, load_renderable_obj(
                            filename,
                            cfg<float>(args, light_configuration),
                            scene_node_resources));
                    } else {
                        scene_node_resources.add_resource(name, load_renderable_obj(
                            filename,
                            cfg<CompressedScenePos>(args, light_configuration),
                            scene_node_resources));
                    }
                } else if (filename.ends_with(".kn5") || filename.ends_with(".ini")) {
                    if (!args.has_named("--large_object_mode")) {
                        scene_node_resources.add_resource(name, load_renderable_kn5(
                            filename,
                            cfg<float>(args, light_configuration),
                            scene_node_resources,
                            &RenderingContextStack::primary_rendering_resources(),
                            nullptr)); // race_logic
                    } else {
                        scene_node_resources.add_resource(name, load_renderable_kn5(
                            filename,
                            cfg<CompressedScenePos>(args, light_configuration),
                            scene_node_resources,
                            &RenderingContextStack::primary_rendering_resources(),
                            nullptr)); // race_logic
                    }
                } else if (filename.ends_with(".dff")) {
                    scene_node_resources.add_resource(name, load_renderable_dff(
                        filename,
                        cfg<float>(args, light_configuration),
                        scene_node_resources,
                        DrawDistanceDb{}));
                } else if (filename.ends_with(".mhx2")) {
                    auto rmhx2 = std::make_shared<Mhx2FileResource>(
                        filename,
                        cfg<float>(args, light_configuration));
                    scene_node_resources.add_resource(name, rmhx2);
                    scene_node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
                        .periodic_skelletal_animation_name = "anim",
                        .periodic_skelletal_animation_frame = {
                            AnimationFrame {
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
                        .magnifying_interpolation_mode = InterpolationMode::NEAREST,
                        .aggregate_mode = AggregateMode::NONE,
                        .transformation_mode = TransformationMode::ALL,
                        .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
                        .apply_static_lighting = false,
                        .laplace_ao_strength = 0.f,
                        .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
                        .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
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
            if (args.has_named("--cleanup_mesh")) {
                add_cleanup_mesh_modifier(
                    "objs",
                    scene_node_resources,
                    0.f,                    // min_vertex_distance
                    PhysicsMaterial::NONE,  // min_distance_filter
                    true);                  // modulo_uv (this computes the material.period_world)
            }
            merge_textures(
                "objs",
                MergedTexturesConfig{
                    .resource_name = "merged_resource",
                    .array_name = "merged_array",
                    .texture_name = ColormapWithModifiers{
                        .filename = VariableAndHash<std::string>{"merged_texture"},
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                        .anisotropic_filtering_level = 0
                    },
                    .blend_mode = BlendMode::SEMI_CONTINUOUS_02,
                    .continuous_blending_z_order = 1,
                    .aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY,
                    .max_triangle_distance = INFINITY,
                    .cull_faces = false,
                    .mip_level_count = 8
                },
                scene_node_resources,
                RenderingContextStack::primary_rendering_resources());
            {
                if (!args.has_named("--hide_object")) {
                    scene_node_resources.instantiate_child_renderable(
                        "objs",
                        ChildInstantiationOptions{
                            .rendering_resources = &rendering_resources,
                            .instance_name = VariableAndHash<std::string>{"objs"},
                            .scene_node = scene_node.ref(DP_LOC),
                            .interpolation_mode = args.has_named("--large_object_mode")
                                ? PoseInterpolationMode::DISABLED
                                : PoseInterpolationMode::ENABLED,
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
                        using I = funpack_t<TPos>;
                        Interp<I> interp{
                            {safe_stox<I>(args.named_value("--color_gradient_min_x")),
                            safe_stox<I>(args.named_value("--color_gradient_max_x"))},
                            {safe_stox<float>(args.named_value("--color_gradient_min_c")),
                            safe_stox<float>(args.named_value("--color_gradient_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = Colors::from_float((float)interp(funpack(v.position(0))));
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
                        using I = funpack_t<TPos>;
                        Interp<I> interp{
                            {safe_stox<I>(args.named_value("--color_radial_min_r")),
                            safe_stox<I>(args.named_value("--color_radial_max_r"))},
                            {safe_stox<float>(args.named_value("--color_radial_min_c")),
                            safe_stox<float>(args.named_value("--color_radial_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        FixedArray<TPos, 3> center{
                            safe_stox<TPos>(args.named_value("--color_radial_center_x", "0")),
                            safe_stox<TPos>(args.named_value("--color_radial_center_y", "0")),
                            safe_stox<TPos>(args.named_value("--color_radial_center_z", "0"))};
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = Colors::from_float((float)interp(std::sqrt(sum(squared(v.position - center)))));
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
                        using I = funpack_t<TPos>;
                        Interp<I> interp{
                            {safe_stox<I>(args.named_value("--color_cone_min_r")),
                            safe_stox<I>(args.named_value("--color_cone_max_r"))},
                            {safe_stox<float>(args.named_value("--color_cone_min_c")),
                            safe_stox<float>(args.named_value("--color_cone_max_c"))},
                            OutOfRangeBehavior::CLAMP};
                        I bottom = safe_stox<I>(args.named_value("--color_cone_bottom", "0"));
                        I top = safe_stox<I>(args.named_value("--color_cone_top"));
                        I cx = safe_stox<I>(args.named_value("--color_cone_x", "0"));
                        I cz = safe_stox<I>(args.named_value("--color_cone_z", "0"));
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    auto p = funpack(v.position);
                                    I r = std::sqrt(squared(p(0) - cx) + squared(p(2) - cz));
                                    I h = (top - p(1)) / (top - bottom);
                                    v.color = Colors::from_float((float)interp(r / h));
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
                    FixedArray<float, 3> color{
                        safe_stox<float>(args.named_value("--color_r", "-1")),
                        safe_stox<float>(args.named_value("--color_g", "-1")),
                        safe_stox<float>(args.named_value("--color_b", "-1"))};
                    if (any(color != -1.f)) {
                        for (auto& m : cvas) {
                            for (auto& t : m->triangles) {
                                for (auto& v : t.flat_iterable()) {
                                    v.color = Colors::from_rgb(maximum(color.template casted<float>(), 0.f));
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
        scene.auto_add_root_node(
            "obj",
            std::move(scene_node),
            args.has_named("--large_object_mode")
                ? RenderingDynamics::STATIC
                : RenderingDynamics::MOVING);

        size_t light_beacon_index = 0;
        auto add_light_beacon_if_set = [&](DanglingRef<SceneNode> scene_node){
            if (!args.has_named_value("--light_beacon")) {
                return;
            }
            auto name = VariableAndHash{ "light_beacon-" + std::to_string(light_beacon_index++) };
            LoadMeshConfig<float> cfg{
                .position = fixed_zeros<float, 3>(),
                .rotation = fixed_zeros<float, 3>(),
                .scale = fixed_full<float, 3>(safe_stof(args.named_value("--light_beacon_scale", "1"))),
                .blend_mode = blend_mode_from_string(args.named_value("--blend_mode", "binary_05")),
                .cull_faces_default = !args.has_named("--no_cull_faces_default"),
                .cull_faces_alpha = args.has_named("--cull_faces_alpha"),
                .occluded_pass = ExternalRenderPassType::NONE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .magnifying_interpolation_mode = InterpolationMode::NEAREST,
                .aggregate_mode = AggregateMode::NONE,
                .transformation_mode = TransformationMode::ALL,
                .triangle_tangent_error_behavior = triangle_tangent_error_behavior_from_string(args.named_value("--triangle_tangent_error_behavior", "warn")),
                .apply_static_lighting = args.has_named("--apply_static_lighting"),
                .laplace_ao_strength = 0.f,
                .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
                .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
                .werror = !args.has_named("--no_werror")};
            scene_node_resources.add_resource(*name, load_renderable_obj(
                args.named_value("--light_beacon"),
                cfg,
                scene_node_resources));
            scene_node_resources.instantiate_child_renderable(
                *name,
                ChildInstantiationOptions{
                    .instance_name = name,
                    .scene_node = scene_node,
                    .interpolation_mode = args.has_named("--large_object_mode")
                        ? PoseInterpolationMode::DISABLED
                        : PoseInterpolationMode::ENABLED,
                    .renderable_resource_filter = RenderableResourceFilter{}});
        };
        std::list<LightAndNode> lights;
        SelectedCameras selected_cameras{scene};
        if (light_configuration == "one") {
            scene.add_root_node(
                "light_node0",
                make_unique_scene_node(
                    FixedArray<ScenePos, 3>{
                        safe_stof(args.named_value("--light_x", "0")),
                        safe_stof(args.named_value("--light_y", "50")),
                        safe_stof(args.named_value("--light_z", "0"))},
                    FixedArray<float, 3>{
                        safe_stof(args.named_value("--light_angle_x", "-45")) * degrees,
                        safe_stof(args.named_value("--light_angle_y", "0")) * degrees,
                        safe_stof(args.named_value("--light_angle_z", "0")) * degrees},
                    1.f),
                RenderingDynamics::MOVING,
                RenderingStrategies::OBJECT);
            auto light = create_light("light_node0");
            lights.push_back({.light = light, .node = scene.get_node("light_node0", DP_LOC)});
            scene.get_node("light_node0", DP_LOC)->add_light(std::move(light));
            scene.get_node("light_node0", DP_LOC)->set_camera(
                std::make_unique<PerspectiveCamera>(
                    PerspectiveCameraConfig(),
                    PerspectiveCamera::Postprocessing::ENABLED));
            add_light_beacon_if_set(scene.get_node("light_node0", DP_LOC));
        } else if (light_configuration == "circle" || light_configuration == "shifted_circle") {
            size_t n = 10;
            float r = 50;
            bool with_diffusivity = true;
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
                    THROW_OR_ABORT("Could not compute lookat for light " + std::to_string(i));
                }
                scene.add_root_node(
                    name,
                    make_unique_scene_node(
                        FixedArray<ScenePos, 3>{r * std::cos(a) + center(0), center(1), r * std::sin(a) + center(2)},
                        matrix_2_tait_bryan_angles(*R).casted<float>(),
                        1.f),
                    RenderingDynamics::MOVING,
                    RenderingStrategies::OBJECT);
                auto light = create_light(name);
                lights.push_back({.light = light, .node = scene.get_node(name, DP_LOC)});
                scene.get_node(name, DP_LOC)->add_light(light);
                scene.get_node(name, DP_LOC)->set_camera(
                    std::make_unique<PerspectiveCamera>(
                        PerspectiveCameraConfig(),
                        PerspectiveCamera::Postprocessing::ENABLED));
                light->ambient *= 2.f / float(n * size_t(1 + (int)with_diffusivity));
                light->diffuse = 0.f;
                light->specular = 0.f;
            }
            if (with_diffusivity) {
                for (const auto& [i, a] : enumerate(Linspace<float>(0.f, 2.f * float(M_PI), n))) {
                    std::string name = "light_s" + std::to_string(i);
                    auto R = gl_lookat_absolute(
                        scene.get_node(name, DP_LOC)->position(),
                        scene.get_node("obj", DP_LOC)->position());
                    if (!R.has_value()) {
                        THROW_OR_ABORT("Could not compute lookat for light " + std::to_string(i));
                    }
                    scene.add_root_node(
                        name,
                        make_unique_scene_node(
                            FixedArray<ScenePos, 3>{r * std::cos(a) + center(0), center(1), r * std::sin(a) + center(2)},
                            matrix_2_tait_bryan_angles(*R).casted<float>(),
                            1.f),
                        RenderingDynamics::MOVING,
                        RenderingStrategies::OBJECT);
                    auto light = create_light(name);
                    lights.push_back({.light = light, .node = scene.get_node(name, DP_LOC)});
                    scene.get_node(name, DP_LOC)->add_light(std::move(light));
                    scene.get_node(name, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                        PerspectiveCameraConfig(),
                        PerspectiveCamera::Postprocessing::ENABLED));
                    light->ambient = 0.f;
                    light->diffuse /= (float)(2 * n);
                    light->specular = 0.f;
                }
            }
        } else if ((light_configuration != "none") && (light_configuration != "emissive")) {
            throw std::runtime_error("Unknown light configuration");
        }
        if (args.has_named_value("--background_light_ambience")) {
            std::string name = "background_light";
            scene.add_root_node(
                name,
                make_unique_scene_node(),
                RenderingDynamics::MOVING,
                RenderingStrategies::OBJECT);
            auto light = create_light(name);
            lights.push_back({.light = light, .node = scene.get_node(name, DP_LOC)});
            scene.get_node(name, DP_LOC)->add_light(std::move(light));
            scene.get_node(name, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                PerspectiveCameraConfig(),
                PerspectiveCamera::Postprocessing::ENABLED));
            light->ambient = FixedArray<float, 3>{1.f, 1.f, 1.f} * safe_stof(args.named_value("--background_light_ambience"));
            light->diffuse = 0.f;
            light->specular = 0.f;
        }
        
        if (args.has_named("--look_at_aabb")) {
            auto aabb = scene.get_node("obj", DP_LOC)->relative_aabb();
            if (aabb.empty()) {
                throw std::runtime_error("Node has an empty AABB");
            }
            if (aabb.full()) {
                throw std::runtime_error("Node has a full AABB");
            }
            scene.add_root_node("follower_camera", make_unique_scene_node(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
            auto la = gl_lookat_aabb(
                scene.get_node("follower_camera", DP_LOC)->position(),
                scene.get_node("obj", DP_LOC)->absolute_model_matrix(),
                aabb.data());
            if (!la.has_value()) {
                throw std::runtime_error("Could not compute frustum, camera might be inside the object's AABB");
            }
            auto npixels = npixels_for_dpi(
                la->sensor_aabb,
                PerspectiveCameraConfig().dpi((float)render_config.windowed_height),
                1,
                2048);
            if (!npixels.has_value()) {
                throw std::runtime_error("Could not compute npixels, object might be too small or too large");
            }
            if (args.has_named_value("--output")) {
                render_results.outputs.at(rsd).width = npixels->width;
                render_results.outputs.at(rsd).height = npixels->height;
            }
            scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<FrustumCamera>(
                FrustumCameraConfig::from_sensor_aabb(
                    npixels->scaled_sensor_aabb,
                    la->near_plane,
                    la->far_plane),
                FrustumCamera::Postprocessing::ENABLED));
            scene.get_node("follower_camera", DP_LOC)->set_rotation(
                matrix_2_tait_bryan_angles(la->extrinsic_R),
                std::chrono::steady_clock::time_point());
        } else {
            scene.add_root_node(
                "follower_camera",
                make_unique_scene_node(
                    FixedArray<ScenePos, 3>{
                        safe_stof(args.named_value("--camera_x", "0")),
                        safe_stof(args.named_value("--camera_y", "0")),
                        safe_stof(args.named_value("--camera_z", "0"))},
                    FixedArray<float, 3>{
                        safe_stof(args.named_value("--camera_angle_x", "0")) * degrees,
                        safe_stof(args.named_value("--camera_angle_y", "0")) * degrees,
                        safe_stof(args.named_value("--camera_angle_z", "0")) * degrees},
                    1.f),
                RenderingDynamics::MOVING,
                RenderingStrategies::OBJECT);
            scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
                PerspectiveCameraConfig{
                    .y_fov = safe_stof(args.named_value("--y_fov", "90")) * degrees},
                PerspectiveCamera::Postprocessing::ENABLED));
        }
        
        // scene.print();
        Focuses focuses = { Focus::SCENE };
        ButtonStates button_states;
        CursorStates cursor_states;
        CursorStates scroll_wheel_states;
        StandardCameraLogic standard_camera_logic{
            scene,
            selected_cameras};
        StandardRenderLogic standard_render_logic{
            scene,
            standard_camera_logic,
            {
                safe_stof(args.named_value("--background_r", "1")),
                safe_stof(args.named_value("--background_g", "0")),
                safe_stof(args.named_value("--background_b", "1"))},
            ClearMode::COLOR_AND_DEPTH };
        AggregateRenderLogic aggregate_render_logic{
            rendering_resources,
            standard_render_logic };
        WindowUserClass window_user_object{
            .window_position{
                .fullscreen_width = render_config.fullscreen_width,
                .fullscreen_height = render_config.fullscreen_height,
            },
            .button_states = button_states,
            .exit_on_escape = true};
        WindowLogic window_logic{render.glfw_window(), window_user_object};
        UiFocus ui_focus{""};
        RenderLogics render_logics{ui_focus};
        FlyingCameraUserClass flying_camera_user_object{
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .cameras = selected_cameras,
            .wire_frame = render_config.wire_frame,
            .depth_test = render_config.depth_test,
            .cull_faces = render_config.cull_faces,
            .delete_node_mutex = delete_node_mutex,
            .physics_set_fps = nullptr};
        ObjectPool object_pool{ InObjectPoolDestructor::CLEAR };
        auto& flying_camera_logic = object_pool.create<FlyingCameraLogic>(
            CURRENT_SOURCE_LOCATION,
            scene,
            flying_camera_user_object,
            true,                                       // fly
            !args.has_named("--large_object_mode"));    // rotate
        ReadPixelsLogic read_pixels_logic{
            aggregate_render_logic,
            button_states,
            ReadPixelsRole::INTERMEDIATE | ReadPixelsRole::SCREENSHOT };
        std::list<LightmapLogic*> lightmap_logics;
        for (const auto& l : lights) {
            if (any(l.light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_DEPTH)) {
                lightmap_logics.push_back(&object_pool.create<LightmapLogic>(
                    CURRENT_SOURCE_LOCATION,
                    rendering_resources,
                    read_pixels_logic,
                    l.light->shadow_render_pass,
                    l.node,
                    l.light,
                    "",                                 // black_node_name
                    true,                               // with_depth_texture
                    2048,                               // lightmap_width
                    2048,                               // lightmap_height
                    FixedArray<uint32_t, 2>{0u, 0u}));  // smooth_niterations (not supported for depth textures)
            }
        }

        render_logics.append({ flying_camera_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
        for (const auto& l : lightmap_logics) {
            render_logics.append({ *l, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
        }
        render_logics.append({ read_pixels_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
        // The following is required for animations.
        render_logics.append(
            {
                object_pool.create<MoveSceneLogic>(
                    CURRENT_SOURCE_LOCATION,
                    scene,
                    delete_node_mutex,
                    safe_stof(args.named_value("--speed", "1"))),
                CURRENT_SOURCE_LOCATION
            },
            0 /* z_order */,
            CURRENT_SOURCE_LOCATION);
        LambdaRenderLogic lrl{
            [&render_logics](
                const LayoutConstraintParameters& lx,
                const LayoutConstraintParameters& ly,
                const RenderConfig& render_config,
                const SceneGraphConfig& scene_graph_config,
                RenderResults* render_results,
                const RenderedSceneDescriptor& frame_id)
            {
                execute_render_allocators();
                render_logics.render_toplevel(lx, ly, render_config, scene_graph_config, render_results, frame_id);
            }
        };
        render.render(
            lrl,
            [&window_logic]() { window_logic.handle_events(); },
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
        lerr() << e.what();
        return 1;
    }
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        return 1;
    }
    return 0;
}
