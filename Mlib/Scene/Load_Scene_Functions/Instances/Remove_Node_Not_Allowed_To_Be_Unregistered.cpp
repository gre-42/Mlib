#include "Remove_Node_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction RemoveNodeNotAllowedToBeUnregistered::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*remove_node_not_allowed_to_be_unregistered"
        "\\s+name=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RemoveNodeNotAllowedToBeUnregistered(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RemoveNodeNotAllowedToBeUnregistered::RemoveNodeNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RemoveNodeNotAllowedToBeUnregistered::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.remove_node_not_allowed_to_be_unregistered(match[1].str());
}
