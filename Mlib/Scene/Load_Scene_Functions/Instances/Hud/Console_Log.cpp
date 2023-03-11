#include "Console_Log.hpp"
#include <Mlib/Physics/Advance_Times/Movable_Logger.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LoadSceneUserFunction ConsoleLog::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*console_log"
        "\\s+node=([\\w+-.]+)"
        "\\s+format=([\\w|]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ConsoleLog(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ConsoleLog::ConsoleLog(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ConsoleLog::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[1].str());
    auto lo = dynamic_cast<StatusWriter*>(&node.get_absolute_movable());
    if (lo == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a status writer");
    }
    StatusComponents log_components = status_components_from_string(match[2].str());
    auto logger = std::make_unique<MovableLogger>(
        node,
        physics_engine.advance_times_,
        lo,
        log_components);
    physics_engine.advance_times_.add_advance_time(std::move(logger));
}
