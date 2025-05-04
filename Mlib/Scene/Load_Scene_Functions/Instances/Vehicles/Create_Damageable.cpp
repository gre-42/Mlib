#include "Create_Damageable.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_And_Id.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translator.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
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
DECLARE_ARGUMENT(gid);
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
    global_object_pool.create<DeletingDamageable>(
        CURRENT_SOURCE_LOCATION,
        scene,
        physics_engine.advance_times_,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node),
        args.arguments.at<float>(KnownArgs::health),
        args.arguments.at<bool>(KnownArgs::delete_node_when_health_leq_zero),
        std::make_unique<Translator>(
            args.translators,
            VariableAndHash{args.arguments.at<AssetGroupAndId>(KnownArgs::gid)}));
}
