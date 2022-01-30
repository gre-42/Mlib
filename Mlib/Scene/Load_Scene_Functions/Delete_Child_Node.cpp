#include "Delete_Child_Node.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);

LoadSceneInstanceFunction::UserFunction DeleteChildNode::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*delete_child_node"
        "\\s+name=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        DeleteChildNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

DeleteChildNode::DeleteChildNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteChildNode::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    scene.delete_child_node(match[NAME].str());
}
