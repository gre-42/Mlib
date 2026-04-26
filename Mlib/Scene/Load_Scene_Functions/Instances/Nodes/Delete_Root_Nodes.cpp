#include "Delete_Root_Nodes.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(regex);
}

DeleteRootNodes::DeleteRootNodes(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void DeleteRootNodes::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    scene.delete_root_nodes(Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::regex)));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "delete_root_nodes",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                DeleteRootNodes{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
