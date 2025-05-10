#include "Add_Blend_Map_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json/Blend_Map_Texture_Json.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(parameters);
}

const std::string AddBlendMapTexture::key = "add_blend_map_texture";

LoadSceneJsonUserFunction AddBlendMapTexture::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddBlendMapTexture::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& rr = RenderingContextStack::primary_rendering_resources();
    rr.set_blend_map_texture(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        blend_map_texture_from_json(args.arguments.child(KnownArgs::parameters)));
}
