#include "Delete_Scheduled_Advance_Times.hpp"
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>

using namespace Mlib;

const std::string DeleteScheduledAdvanceTimes::key = "delete_scheduled_advance_times";

LoadSceneUserFunction DeleteScheduledAdvanceTimes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    DeleteScheduledAdvanceTimes(args.renderable_scene()).execute(match, args);
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
