#include "Console_Log.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Status_Writer.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movable_Logger.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(format);
}

const std::string ConsoleLog::key = "console_log";

LoadSceneJsonUserFunction ConsoleLog::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ConsoleLog(args.renderable_scene()).execute(args);
};

ConsoleLog::ConsoleLog(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ConsoleLog::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& lo = get_status_writer(node);
    StatusComponents log_components = status_components_from_string(args.arguments.at<std::string>(KnownArgs::format));
    auto& logger = global_object_pool.create<MovableLogger>(
        CURRENT_SOURCE_LOCATION,
        node,
        lo,
        log_components);
    physics_engine.advance_times_.add_advance_time({ logger, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}
