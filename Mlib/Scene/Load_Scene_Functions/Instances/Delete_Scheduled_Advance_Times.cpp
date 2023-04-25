#include "Delete_Scheduled_Advance_Times.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string DeleteScheduledAdvanceTimes::key = "delete_scheduled_advance_times";

LoadSceneJsonUserFunction DeleteScheduledAdvanceTimes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    DeleteScheduledAdvanceTimes(args.renderable_scene()).execute(args);
};

DeleteScheduledAdvanceTimes::DeleteScheduledAdvanceTimes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteScheduledAdvanceTimes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    physics_engine.advance_times_.delete_scheduled_advance_times();
}
