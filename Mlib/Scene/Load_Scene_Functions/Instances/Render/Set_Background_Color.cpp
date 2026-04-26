#include "Set_Background_Color.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Deferred_Instantiator.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
}

SetBackgroundColor::SetBackgroundColor(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetBackgroundColor::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto background_color = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::color);
    deferred_instantiator.set_background_color(background_color);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_background_color",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetBackgroundColor{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
