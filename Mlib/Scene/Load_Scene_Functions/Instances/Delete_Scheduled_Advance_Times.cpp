#include "Delete_Scheduled_Advance_Times.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction DeleteScheduledAdvanceTimes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*delete_scheduled_advance_times$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        DeleteScheduledAdvanceTimes(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

DeleteScheduledAdvanceTimes::DeleteScheduledAdvanceTimes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteScheduledAdvanceTimes::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    physics_engine.advance_times_.delete_scheduled_advance_times();
}
