#include "Create_Damageable.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(health);
DECLARE_ARGUMENT(delete_node_when_health_leq_zero);
}

const std::string CreateDamageable::key = "damageable";

LoadSceneJsonUserFunction CreateDamageable::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDamageable(args.renderable_scene()).execute(args);
};

CreateDamageable::CreateDamageable(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDamageable::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC)->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    auto d = std::make_unique<DeletingDamageable>(
        scene,
        physics_engine.advance_times_,
        args.arguments.at<std::string>(KnownArgs::node),
        args.arguments.at<float>(KnownArgs::health),
        args.arguments.at<bool>(KnownArgs::delete_node_when_health_leq_zero),
        delete_node_mutex);
    auto& p_d = *d;
    physics_engine.advance_times_.add_advance_time(std::move(d));
    if (rb->damageable_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a damageable");
    }
    rb->damageable_ = &p_d;
}
