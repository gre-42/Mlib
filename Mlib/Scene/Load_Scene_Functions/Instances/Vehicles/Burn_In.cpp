#include "Burn_In.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(seconds);
}

const std::string BurnIn::key = "burn_in";

LoadSceneJsonUserFunction BurnIn::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    BurnIn(args.renderable_scene()).execute(args);
};

BurnIn::BurnIn(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void BurnIn::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    FunctionGuard fg{ "Physics burn-in" };

    StaticWorld world{
        .geographic_mapping = dynamic_world.get_geographic_mapping(),
        .inverse_geographic_mapping = dynamic_world.get_inverse_geographic_mapping(),
        .gravity = dynamic_world.get_gravity(),
        .wind = dynamic_world.get_wind(),
        .time = std::chrono::steady_clock::now()
    };
    physics_engine.burn_in(world, args.arguments.at<float>(KnownArgs::seconds) * seconds);
    scene.move(0.f, std::chrono::steady_clock::now()); // dt
}
