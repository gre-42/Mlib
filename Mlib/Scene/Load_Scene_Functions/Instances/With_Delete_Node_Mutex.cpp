#include "With_Delete_Node_Mutex.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

const std::string WithDeleteNodeMutex::key = "with_delete_node_mutex";

LoadSceneUserFunction WithDeleteNodeMutex::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^([\\s\\S]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    WithDeleteNodeMutex(args.renderable_scene()).execute(match, args);
};

WithDeleteNodeMutex::WithDeleteNodeMutex(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void WithDeleteNodeMutex::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::scoped_lock lock{delete_node_mutex};
    args.macro_line_executor(match[1].str(), nullptr);
}
