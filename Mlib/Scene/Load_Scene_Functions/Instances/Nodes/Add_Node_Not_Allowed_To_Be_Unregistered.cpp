#include "Add_Node_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

const std::string AddNodeNotAllowedToBeUnregistered::key = "add_node_not_allowed_to_be_unregistered";

LoadSceneUserFunction AddNodeNotAllowedToBeUnregistered::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    AddNodeNotAllowedToBeUnregistered(args.renderable_scene()).execute(match, args);
};

AddNodeNotAllowedToBeUnregistered::AddNodeNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddNodeNotAllowedToBeUnregistered::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.add_node_not_allowed_to_be_unregistered(match[1].str());
}
