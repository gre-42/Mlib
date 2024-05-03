#include "Create_Drive_Or_Walk_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Vehicle_Ai/Drive_Or_Walk_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
}

const std::string CreateDriveOrWalkAi::key = "create_drive_or_walk_ai";

LoadSceneJsonUserFunction CreateDriveOrWalkAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDriveOrWalkAi(args.renderable_scene()).execute(args);
};

CreateDriveOrWalkAi::CreateDriveOrWalkAi(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriveOrWalkAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto ai = std::make_unique<DriveOrWalkAi>(player);
    player->rigid_body().add_autopilot({ *ai, CURRENT_SOURCE_LOCATION });
    global_object_pool.add(std::move(ai), CURRENT_SOURCE_LOCATION);
}
