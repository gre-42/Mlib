#include "With_Delete_Node_Mutex.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
}

const std::string WithDeleteNodeMutex::key = "with_delete_node_mutex";

LoadSceneJsonUserFunction WithDeleteNodeMutex::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    WithDeleteNodeMutex(args.renderable_scene()).execute(args);
};

WithDeleteNodeMutex::WithDeleteNodeMutex(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void WithDeleteNodeMutex::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::scoped_lock lock{delete_node_mutex};
    args.macro_line_executor(args.arguments.at(KnownArgs::command), nullptr);
}
