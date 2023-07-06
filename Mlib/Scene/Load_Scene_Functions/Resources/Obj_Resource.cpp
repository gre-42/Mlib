#include "Obj_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(max_triangle_distance);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(cull_faces_default);
DECLARE_ARGUMENT(cull_faces_alpha);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
DECLARE_ARGUMENT(reflection_map);
DECLARE_ARGUMENT(desaturate);
DECLARE_ARGUMENT(histogram);
DECLARE_ARGUMENT(triangle_tangent_error_behavior);
DECLARE_ARGUMENT(physics_material);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(werror);
}

const std::string ObjResource::key = "obj_resource";

LoadSceneJsonUserFunction ObjResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void ObjResource::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (!args.arguments.at<bool>(KnownArgs::double_precision, false)) {
        execute<float>(args);
    } else {
        execute<double>(args);
    }
}

template <class TPos>
void ObjResource::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    LoadMeshConfig<TPos> load_mesh_config{
        .position = args.arguments.at<FixedArray<TPos, 3>>(KnownArgs::position) * (TPos)meters,
        .rotation = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        .scale = args.arguments.at<FixedArray<float, 3>>(KnownArgs::scale),
        .center_distances = OrderableFixedArray<float, 2>{
            args.arguments.at<FixedArray<float, 2>>(
                KnownArgs::center_distances,
                FixedArray<float, 2>{0.f, INFINITY }) * meters},
        .max_triangle_distance = args.arguments.at<float>(KnownArgs::max_triangle_distance, INFINITY) * meters,
        .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)),
        .alpha_distances = args.arguments.at<OrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
        .cull_faces_default = args.arguments.at<bool>(KnownArgs::cull_faces_default, true),
        .cull_faces_alpha = args.arguments.at<bool>(KnownArgs::cull_faces_alpha, true),
        .occluded_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)),
        .occluder_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)),
        .magnifying_interpolation_mode = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::magnifying_interpolation_mode, "nearest")),
        .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
        .transformation_mode = transformation_mode_from_string(args.arguments.at<std::string>(KnownArgs::transformation_mode)),
        .reflection_map = args.arguments.at<std::string>(KnownArgs::reflection_map, ""),
        .desaturate = args.arguments.at<bool>(KnownArgs::desaturate, false),
        .histogram = args.arguments.try_path_or_variable(KnownArgs::histogram).path,
        .triangle_tangent_error_behavior = args.arguments.contains(KnownArgs::triangle_tangent_error_behavior)
            ? triangle_tangent_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::triangle_tangent_error_behavior))
            : TriangleTangentErrorBehavior::RAISE,
        .apply_static_lighting = false,
        .laplace_ao_strength = 0.f,
        .physics_material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::physics_material, "attr_visible|attr_collide")),
        .werror = args.arguments.at<bool>(KnownArgs::werror, true)};
    std::string filename = args.arguments.try_path_or_variable(KnownArgs::filename).path;
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    if (filename.ends_with(".obj")) {
        scene_node_resources.add_resource_loader(
            args.arguments.at<std::string>(KnownArgs::name),
            [filename, load_mesh_config, &scene_node_resources](){
                return load_renderable_obj(
                    filename,
                    load_mesh_config,
                    scene_node_resources);
            });
    } else if (filename.ends_with(".mhx2")) {
        if constexpr (std::is_same_v<TPos, float>) {
            scene_node_resources.add_resource_loader(
                args.arguments.at<std::string>(KnownArgs::name),
                [filename, load_mesh_config](){
                    return std::make_shared<Mhx2FileResource>(
                        filename,
                        load_mesh_config);
                });
        } else {
            THROW_OR_ABORT("MHX2 does not support double precision");
        }
    } else {
        THROW_OR_ABORT("Unknown file type: " + filename);
    }
}
