#include "Delete_Root_Nodes.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(REGEX);

const std::string DeleteRootNodes::key = "delete_root_nodes";

LoadSceneUserFunction DeleteRootNodes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^regex=(.*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    DeleteRootNodes(args.renderable_scene()).execute(match, args);
};

DeleteRootNodes::DeleteRootNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteRootNodes::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::scoped_lock node_lock{ delete_node_mutex };
    scene.delete_root_nodes(Mlib::compile_regex(match[REGEX].str()));
}
