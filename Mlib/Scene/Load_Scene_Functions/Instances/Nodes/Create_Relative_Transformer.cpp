#include "Create_Relative_Transformer.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(v);
DECLARE_ARGUMENT(w);
}

const std::string CreateRelativeTransformer::key = "relative_transformer";

LoadSceneJsonUserFunction CreateRelativeTransformer::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRelativeTransformer(args.physics_scene()).execute(args);
};

CreateRelativeTransformer::CreateRelativeTransformer(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateRelativeTransformer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto v = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * meters / seconds;
    auto w = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * rpm;
    auto rt = std::make_unique<RelativeTransformer>(v, w);
    linker.link_relative_movable<RelativeTransformer>(
        scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC),
        { *rt, CURRENT_SOURCE_LOCATION },
        CURRENT_SOURCE_LOCATION);
    global_object_pool.add(std::move(rt), CURRENT_SOURCE_LOCATION);
}
