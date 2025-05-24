#include "Create_Copy_Rotation.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Copy_Rotation.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(from);
DECLARE_ARGUMENT(to);
}

const std::string CreateCopyRotation::key = "copy_rotation";

LoadSceneJsonUserFunction CreateCopyRotation::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCopyRotation(args.physics_scene()).execute(args);
};

CreateCopyRotation::CreateCopyRotation(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateCopyRotation::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> from = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::from), DP_LOC);
    DanglingRef<SceneNode> to = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::to), DP_LOC);
    auto rt = std::make_unique<CopyRotation>(from);
    linker.link_relative_movable<CopyRotation>(to, { *rt, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    from->clearing_observers.add({ *rt, CURRENT_SOURCE_LOCATION });
    global_object_pool.add(std::move(rt), CURRENT_SOURCE_LOCATION);
}
