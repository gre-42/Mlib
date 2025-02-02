#include "Set_Soft_Light.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
}

const std::string SetSoftLight::key = "set_soft_light";

LoadSceneJsonUserFunction SetSoftLight::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetSoftLight(args.renderable_scene()).execute(args);
};

SetSoftLight::SetSoftLight(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSoftLight::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    post_processing_logic.set_soft_light_filename(VariableAndHash{ args.arguments.path(KnownArgs::filename) });
}
