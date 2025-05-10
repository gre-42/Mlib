#include "Create_Square_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance_Json.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture_filename);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(emissive);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(depth_func);
DECLARE_ARGUMENT(depth_test);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(cull_faces);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(translation);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
DECLARE_ARGUMENT(number_of_frames);
DECLARE_ARGUMENT(billboards);
DECLARE_ARGUMENT(fog_distances);
DECLARE_ARGUMENT(fog_ambient);
}

const std::string CreateSquareResource::key = "square_resource";

LoadSceneJsonUserFunction CreateSquareResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto billboard_atlas_instances = args.arguments.at<std::vector<BillboardAtlasInstance>>(KnownArgs::billboards, {});
    auto min = args.arguments.at<UFixedArray<float, 2>>(KnownArgs::min) * meters;
    auto max = args.arguments.at<UFixedArray<float, 2>>(KnownArgs::max) * meters;
    auto square = FixedArray<float, 2, 2>::init(
        min(0), min(1),
        max(0), max(1));
    TransformationMatrix<float, float, 3> transformation(
        tait_bryan_angles_2_matrix(
            args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation, fixed_zeros<float, 3>()) * degrees),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::translation, fixed_zeros<float, 3>()) * meters);
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    Material material{
        .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)),
        .continuous_blending_z_order = args.arguments.at<int>(KnownArgs::z_order, 0),
        .depth_func = args.arguments.contains(KnownArgs::depth_func)
            ? depth_func_from_string(args.arguments.at<std::string>(KnownArgs::depth_func))
            : DepthFunc::LESS,
        .depth_test = args.arguments.at<bool>(KnownArgs::depth_test, true),
        .textures_color = { primary_rendering_resources.get_blend_map_texture(VariableAndHash{args.arguments.path_or_variable(KnownArgs::texture_filename).path}) },
        .occluded_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)),
        .occluder_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)),
        .alpha_distances = args.arguments.at<UOrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
        // .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
        // .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
        .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
        .transformation_mode = transformation_mode_from_string(args.arguments.at<std::string>(KnownArgs::transformation_mode)),
        .billboard_atlas_instances = billboard_atlas_instances,
        .number_of_frames = args.arguments.at<unsigned int>(KnownArgs::number_of_frames, 1),
        .cull_faces = args.arguments.at<bool>(KnownArgs::cull_faces),
        .shading{
            .emissive = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::emissive, OrderableFixedArray<float, 3>(0.f)),
            .ambient = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::ambient),
            .diffuse = {0.f, 0.f, 0.f},
            .specular = {0.f, 0.f, 0.f},
            .fog_distances = args.arguments.at<UOrderableFixedArray<float, 2>>(KnownArgs::fog_distances, default_step_distances),
            .fog_ambient= args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::fog_ambient, UOrderableFixedArray<float, 3>(0.f))}};
    material.compute_color_mode();
    Morphology morphology{
        .physics_material = PhysicsMaterial::NONE,
        .center_distances2 = SquaredStepDistances::from_distances(
            args.arguments.at<UFixedArray<float, 2>>(
                KnownArgs::center_distances,
                FixedArray<float, 2>{0.f, INFINITY }) * meters),
    };
    RenderingContextStack::primary_scene_node_resources().add_resource_loader(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        [square, transformation, material, morphology](){
            return std::make_shared<SquareResource>(
                square,
                FixedArray<float, 2, 2>::init(0.f, 0.f, 1.f, 1.f),
                transformation,
                material,
                morphology);});
};
