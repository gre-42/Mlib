#include "Move_Node_To_Bvh.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

MoveNodeToBvh::MoveNodeToBvh(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void MoveNodeToBvh::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    scene.move_root_node_to_bvh(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "move_node_to_bvh",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                MoveNodeToBvh{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
