#include "Create_Blending_X_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Blending_X_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture_filename);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(number_of_frames);
}

const std::string CreateBlendingXResource::key = "blending_x_resource";

LoadSceneJsonUserFunction CreateBlendingXResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto min = args.arguments.at<UFixedArray<float, 2>>(KnownArgs::min);
    auto max = args.arguments.at<UFixedArray<float, 2>>(KnownArgs::max);
    auto square = FixedArray<float, 2, 2>::init(
        min(0), min(1),
        max(0), max(1));
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    Material material{
        .blend_mode = BlendMode::CONTINUOUS,
        .textures_color = { primary_rendering_resources.get_blend_map_texture(VariableAndHash{args.arguments.path_or_variable(KnownArgs::texture_filename).path}) },
        .occluder_pass = ExternalRenderPassType::NONE,
        // .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
        // .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
        .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
        .number_of_frames = args.arguments.at<size_t>(KnownArgs::number_of_frames, 1),
        .cull_faces = false,
        .shading{
            .ambient = {2.f, 2.f, 2.f},
            .diffuse = {0.f, 0.f, 0.f},
            .specular = {0.f, 0.f, 0.f}}};
    material.compute_color_mode();
    Morphology morphology{ .physics_material = PhysicsMaterial::NONE };
    RenderingContextStack::primary_scene_node_resources().add_resource_loader(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        [square, material, morphology](){return std::make_shared<BlendingXResource>(
            square,
            FixedArray<Material, 2>{
                material,
                material },
            FixedArray<Morphology, 2>{
                morphology,
                morphology});});
};
