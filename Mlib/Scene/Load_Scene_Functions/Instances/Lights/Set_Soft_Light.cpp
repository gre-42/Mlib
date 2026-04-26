#include "Set_Soft_Light.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
}

SetSoftLight::SetSoftLight(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void SetSoftLight::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    post_processing_logic.set_soft_light_filename(args.arguments.path_or_variable(KnownArgs::filename));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_soft_light",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetSoftLight{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
