#include "Delete_Root_Nodes.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

LoadSceneUserFunction DeleteRootNodes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*delete_root_nodes"
        "\\s+regex=(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        DeleteRootNodes(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

DeleteRootNodes::DeleteRootNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteRootNodes::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard node_lock{ delete_node_mutex };
    scene.delete_root_nodes(Mlib::compile_regex(match[1].str()));
}
