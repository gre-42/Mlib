#include "Try_Delete_Root_Node.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

TryDeleteRootNode::TryDeleteRootNode(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void TryDeleteRootNode::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    scene.try_delete_root_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "try_delete_root_node",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                TryDeleteRootNode{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
