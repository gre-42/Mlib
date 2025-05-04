#include "Append_Externals_Deleter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
}

AppendExternalsDeleter::AppendExternalsDeleter(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AppendExternalsDeleter::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    DanglingRef<SceneNode> node = scene.get_node(node_name, DP_LOC);
    // players.get_player(args.arguments.at<std::string>(KnownArgs::player)).append_delete_externals(
    //     node.ptr(),
    //     [&scene = scene, node_name]()
    //     {
    //         try {
    //             scene.delete_node(node_name);
    //         } catch (const std::runtime_error& e) {
    //             throw std::runtime_error("Could not delete node \"" + node_name + "\": " + e.what());
    //         }
    //     }
    // );
    players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION)->append_dependent_node(node_name);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "append_externals_deleter",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                AppendExternalsDeleter(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
