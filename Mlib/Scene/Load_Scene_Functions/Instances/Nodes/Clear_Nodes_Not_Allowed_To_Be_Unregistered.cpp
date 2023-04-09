#include "Clear_Nodes_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

const std::string ClearNodesNotAllowedToBeUnregistered::key = "clear_nodes_not_allowed_to_be_unregistered";

LoadSceneUserFunction ClearNodesNotAllowedToBeUnregistered::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    ClearNodesNotAllowedToBeUnregistered(args.renderable_scene()).execute(match, args);
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
