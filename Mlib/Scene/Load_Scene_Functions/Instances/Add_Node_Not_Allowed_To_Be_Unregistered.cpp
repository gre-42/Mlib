#include "Add_Node_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction AddNodeNotAllowedToBeUnregistered::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_node_not_allowed_to_be_unregistered"
        "\\s+name=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        AddNodeNotAllowedToBeUnregistered(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

AddNodeNotAllowedToBeUnregistered::AddNodeNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddNodeNotAllowedToBeUnregistered::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.add_node_not_allowed_to_be_unregistered(match[1].str());
}
