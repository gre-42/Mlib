#include "Print_Camera_Cycles.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

PrintCameraCycle::PrintCameraCycle(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void PrintCameraCycle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    selected_cameras.print_camera_cycles(linfo().ref());
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "print_camera_cycles",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                PrintCameraCycle(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
