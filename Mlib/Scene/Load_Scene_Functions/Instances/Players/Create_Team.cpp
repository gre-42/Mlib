#include "Create_Team.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(name);
}

CreateTeam::CreateTeam(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateTeam::execute(const JsonView& args)
{
    args.validate(KnownArgs::options);
    players.add_team(
        args.at<NTeamCountType>(KnownArgs::id),
        args.at<VariableAndHash<std::string>>(KnownArgs::name));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "team_create",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateTeam(args.physics_scene()).execute(args.arguments);
            });
    }
} obj;

}
