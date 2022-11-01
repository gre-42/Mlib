#include "Try_Delete_Root_Node.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);

LoadSceneUserFunction TryDeleteRootNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*try_delete_root_node"
        "\\s+name=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        TryDeleteRootNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

TryDeleteRootNode::TryDeleteRootNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void TryDeleteRootNode::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard node_lock{ delete_node_mutex };
    scene.try_delete_root_node(match[NAME].str());
}
