#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resources/Binary_X_Resource.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture_filename_0);
DECLARE_ARGUMENT(texture_filename_90);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(cull_faces);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "binary_x_resource",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto min = args.arguments.at<EFixedArray<float, 2>>(KnownArgs::min) * meters;
                auto max = args.arguments.at<EFixedArray<float, 2>>(KnownArgs::max) * meters;
                auto square = FixedArray<float, 2, 2>::init(
                    min(0), min(1),
                    max(0), max(1));
                auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
                Material material{
                    .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)),
                    .occluded_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)),
                    .occluder_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)),
                    .alpha_distances = args.arguments.at<EOrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
                    // .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
                    // .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
                    .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
                    .transformation_mode = transformation_mode_from_string(args.arguments.at<std::string>(KnownArgs::transformation_mode)),
                    .cull_faces = args.arguments.at<bool>(KnownArgs::cull_faces),
                    .shading{
                        .ambient = args.arguments.at<EOrderableFixedArray<float, 3>>(KnownArgs::ambient),
                        .diffuse = {0.f, 0.f, 0.f},
                        .specular = {0.f, 0.f, 0.f}}};
                Morphology morphology{
                    .physics_material = PhysicsMaterial::NONE,
                    .center_distances2 = SquaredStepDistances::from_distances(
                        args.arguments.at<EFixedArray<float, 2>>(
                            KnownArgs::center_distances,
                            FixedArray<float, 2>{0.f, INFINITY })* meters)
                };
                Material material_0{material};
                Material material_90{material};
                material_0.textures_color = { primary_rendering_resources.get_blend_map_texture(args.arguments.path_or_variable(KnownArgs::texture_filename_0)) };
                material_90.textures_color = { primary_rendering_resources.get_blend_map_texture(args.arguments.path_or_variable(KnownArgs::texture_filename_90)) };
                material_0.compute_color_mode();
                material_90.compute_color_mode();
                RenderingContextStack::primary_scene_node_resources().add_resource_loader(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [square, material_0, material_90, morphology](){return std::make_shared<BinaryXResource>(
                        square,
                        material_0,
                        material_90,
                        morphology,
                        morphology);});
            });
    }
} obj;

}
