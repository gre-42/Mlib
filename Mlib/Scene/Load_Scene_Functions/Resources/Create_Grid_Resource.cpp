#include "Create_Grid_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Grid_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture_filename);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(emissivity);
DECLARE_ARGUMENT(ambience);
DECLARE_ARGUMENT(diffusivity);
DECLARE_ARGUMENT(specularity);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(depth_func);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(cull_faces);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(translation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(uv_scale);
DECLARE_ARGUMENT(period);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
}

const std::string CreateGridResource::key = "grid_resource";

LoadSceneJsonUserFunction CreateGridResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();

    RenderingContextStack::primary_scene_node_resources().add_resource(args.arguments.at<std::string>(KnownArgs::name), std::make_shared<GridResource>(
        args.arguments.at<FixedArray<size_t, 2>>(KnownArgs::size),
        TransformationMatrix<float, double, 3>(
            tait_bryan_angles_2_matrix(args.arguments.at<FixedArray<float, 3>>(
                KnownArgs::rotation,
                fixed_zeros<float, 3>()) * degrees),
            args.arguments.at<FixedArray<double, 3>>(
                KnownArgs::translation,
                fixed_zeros<double, 3>()) * (double)meters),
        args.arguments.at<double>(KnownArgs::scale),
        args.arguments.at<double>(KnownArgs::uv_scale),
        args.arguments.at<double>(KnownArgs::period),
        Material{
            .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)),
            .depth_func = args.arguments.contains(KnownArgs::depth_func)
                ? depth_func_from_string(args.arguments.at<std::string>(KnownArgs::depth_func))
                : DepthFunc::LESS,
            .textures = {primary_rendering_resources->get_blend_map_texture(args.arguments.path_or_variable(KnownArgs::texture_filename).path)},
            .occluded_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)),
            .occluder_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)),
            .alpha_distances = args.arguments.at<OrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
            .wrap_mode_s = WrapMode::REPEAT,
            .wrap_mode_t = WrapMode::REPEAT,
            .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
            .transformation_mode = transformation_mode_from_string(args.arguments.at<std::string>(KnownArgs::transformation_mode)),
            .center_distances = args.arguments.at<OrderableFixedArray<float, 2>>(
                KnownArgs::center_distances,
                OrderableFixedArray<float, 2>{0.f, INFINITY }),
            .cull_faces = args.arguments.at<bool>(KnownArgs::cull_faces),
            .emissivity = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::emissivity, OrderableFixedArray<float, 3>(0.f)),
            .ambience = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::ambience),
            .diffusivity = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::diffusivity),
            .specularity = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::specularity)
            }.compute_color_mode()));
};
