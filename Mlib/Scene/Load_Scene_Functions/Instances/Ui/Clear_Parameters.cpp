#include "Clear_Parameters.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

ClearParameters::ClearParameters(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ClearParameters::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    throw std::runtime_error("clear_parameters is not implemented");
    // args.external_substitutions.clear_and_notify();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "clear_parameters",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ClearParameters{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
