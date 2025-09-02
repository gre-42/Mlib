#include "Execute_If_Node_Exists.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(command);
}

ExecuteIfNodeExists::ExecuteIfNodeExists(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ExecuteIfNodeExists::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (scene.contains_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node))) {
        args.macro_line_executor(args.arguments.at(KnownArgs::command), nullptr);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "execute_if_node_exists",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                ExecuteIfNodeExists(args.physics_scene()).execute(args);
            });
    }
} obj;

}
