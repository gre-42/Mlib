#include "Clear_Absolute_Observer_And_Notify_Destroyed.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

const std::string ClearAbsoluteObserverAndNotifyDestroyed::key = "clear_absolute_observer_and_notify_destroyed";

LoadSceneJsonUserFunction ClearAbsoluteObserverAndNotifyDestroyed::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearAbsoluteObserverAndNotifyDestroyed(args.renderable_scene()).execute(args);
};

ClearAbsoluteObserverAndNotifyDestroyed::ClearAbsoluteObserverAndNotifyDestroyed(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearAbsoluteObserverAndNotifyDestroyed::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.get_node(args.arguments.at<std::string>(KnownArgs::node)).clear_absolute_observer_and_notify_destroyed();
}
