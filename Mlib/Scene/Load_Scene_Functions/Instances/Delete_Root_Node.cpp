#include "Delete_Root_Node.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction DeleteRootNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*delete_root_node"
        "\\s+name=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        DeleteRootNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

DeleteRootNode::DeleteRootNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteRootNode::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard node_lock{ delete_node_mutex };
    scene.delete_root_node(match[1].str());
}
