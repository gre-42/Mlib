#include "Burn_In.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Time.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(seconds);
}

BurnIn::BurnIn(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void BurnIn::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    FunctionGuard fg{ "Physics burn-in" };

    StaticWorld world{
        .geographic_mapping = dynamic_world.get_geographic_mapping(),
        .inverse_geographic_mapping = dynamic_world.get_inverse_geographic_mapping(),
        .gravity = dynamic_world.get_gravity(),
        .wind = dynamic_world.get_wind(),
        .time = std::chrono::steady_clock::time_point()
    };
    physics_engine.burn_in(world, args.arguments.at<float>(KnownArgs::seconds) * seconds);
    scene.move(0.f, SceneTime::initial()); // dt
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "burn_in",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                BurnIn{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
