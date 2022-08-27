#include "With_Delete_Node_Mutex.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

LoadSceneUserFunction WithDeleteNodeMutex::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*with_delete_node_mutex"
        "\\s+([\\s\\S]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        WithDeleteNodeMutex(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

WithDeleteNodeMutex::WithDeleteNodeMutex(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void WithDeleteNodeMutex::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard lock{delete_node_mutex};
    args.macro_line_executor(match[1].str(), nullptr, args.rsc);
}
