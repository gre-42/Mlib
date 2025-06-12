#include "Create_Internals.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(seat);
}

const std::string CreateInternals::key = "create_internals";

LoadSceneJsonUserFunction CreateInternals::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateInternals(args.physics_scene()).execute(args);
};

CreateInternals::CreateInternals(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateInternals::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->create_vehicle_internals({ args.arguments.at<std::string>(KnownArgs::seat) });
}
