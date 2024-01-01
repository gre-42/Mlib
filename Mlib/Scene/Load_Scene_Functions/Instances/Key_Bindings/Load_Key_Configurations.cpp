#include "Load_Key_Configurations.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(fallback_filename);
}

const std::string LoadKeyConfigurations::key = "load_key_configurations";

LoadSceneJsonUserFunction LoadKeyConfigurations::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    LoadKeyConfigurations(args.renderable_scene()).execute(args);
};

LoadKeyConfigurations::LoadKeyConfigurations(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void LoadKeyConfigurations::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_configurations.load(
        args.arguments.path(KnownArgs::filename),
        args.arguments.path(KnownArgs::fallback_filename));
}
