#include "Clear_Nodes_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction ClearNodesNotAllowedToBeUnregistered::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*clear_nodes_not_allowed_to_be_unregistered$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ClearNodesNotAllowedToBeUnregistered(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ClearNodesNotAllowedToBeUnregistered::ClearNodesNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearNodesNotAllowedToBeUnregistered::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.clear_nodes_not_allowed_to_be_unregistered();
}
