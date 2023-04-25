#include "Console_Log.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movable_Logger.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
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
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto lo = dynamic_cast<StatusWriter*>(&node.get_absolute_movable());
    if (lo == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a status writer");
    }
    StatusComponents log_components = status_components_from_string(args.arguments.at<std::string>(KnownArgs::format));
    auto logger = std::make_unique<MovableLogger>(
        node,
        physics_engine.advance_times_,
        lo,
        log_components);
    physics_engine.advance_times_.add_advance_time(std::move(logger));
}
