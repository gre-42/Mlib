#include "Clear_Parameters.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string ClearParameters::key = "clear_parameters";

LoadSceneJsonUserFunction ClearParameters::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    ClearParameters(args.physics_scene()).execute(args);
};

ClearParameters::ClearParameters(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ClearParameters::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    THROW_OR_ABORT("clear_parameters is not implemented");
    // args.external_substitutions.clear_and_notify();
}
