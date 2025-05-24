#include "Clear_Skybox.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

const std::string ClearSkybox::key = "clear_skybox";

LoadSceneJsonUserFunction ClearSkybox::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearSkybox(args.renderable_scene()).execute(args);
};

ClearSkybox::ClearSkybox(RenderableScene& renderable_scene) 
: LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void ClearSkybox::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    skybox_logic.clear_alias();
}
