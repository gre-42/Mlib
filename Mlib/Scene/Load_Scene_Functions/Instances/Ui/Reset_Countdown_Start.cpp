#include "Reset_Countdown_Start.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Countdown_Physics.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(duration);
}

ResetCountdown::ResetCountdown(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction(physics_scene)
{}

void ResetCountdown::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    countdown_start.reset(args.arguments.at<float>(KnownArgs::duration) * seconds);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "reset_countdown_start",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ResetCountdown(args.physics_scene()).execute(args);
            });
    }
} obj;

}
