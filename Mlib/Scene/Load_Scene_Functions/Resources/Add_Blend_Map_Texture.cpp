#include "Add_Blend_Map_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(min_height);
DECLARE_ARGUMENT(max_height);
DECLARE_ARGUMENT(distances);
DECLARE_ARGUMENT(normal);
DECLARE_ARGUMENT(cosine);
DECLARE_ARGUMENT(discreteness);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(weight);
DECLARE_ARGUMENT(role);
}

const std::string AddBlendMapTexture::key = "add_blend_map_texture";

LoadSceneJsonUserFunction AddBlendMapTexture::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddBlendMapTexture::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto rr = RenderingContextStack::primary_rendering_resources();
    auto tex = args.arguments.path_or_variable(KnownArgs::texture);
    rr->set_blend_map_texture(
        args.arguments.at<std::string>(KnownArgs::name),
        BlendMapTexture{
            .texture_descriptor = tex.is_variable
                ? RenderingContextStack::primary_rendering_resources()->get_existing_texture_descriptor(tex.path)
                : TextureDescriptor{ .color = {.filename = tex.path} },
            .min_height = args.arguments.at<float>(KnownArgs::min_height),
            .max_height = args.arguments.at<float>(KnownArgs::max_height),
            .distances = args.arguments.at<OrderableFixedArray<float, 4>>(KnownArgs::distances),
            .normal = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::normal),
            .cosines = args.arguments.at<OrderableFixedArray<float, 4>>(KnownArgs::cosine),
            .discreteness = args.arguments.at<float>(KnownArgs::discreteness, 2.f),
            .scale = args.arguments.at<float>(KnownArgs::scale),
            .weight = args.arguments.at<float>(KnownArgs::weight),
            .role = blend_map_role_from_string(args.arguments.at<std::string>(KnownArgs::role, "summand")) });
}
