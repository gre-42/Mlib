#include "Set_Skybox.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(alias);
}

const std::string SetSkybox::key = "set_skybox";

LoadSceneJsonUserFunction SetSkybox::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetSkybox(args.renderable_scene()).execute(args);
};

SetSkybox::SetSkybox(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSkybox::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    skybox_logic.set_alias(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::alias));
}
