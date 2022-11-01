#include "Clear_Absolute_Observer_And_Notify_Destroyed.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

LoadSceneUserFunction ClearAbsoluteObserverAndNotifyDestroyed::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*clear_absolute_observer_and_notify_destroyed"
        "\\s+node=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ClearAbsoluteObserverAndNotifyDestroyed(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ClearAbsoluteObserverAndNotifyDestroyed::ClearAbsoluteObserverAndNotifyDestroyed(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearAbsoluteObserverAndNotifyDestroyed::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.get_node(match[NODE].str()).clear_absolute_observer_and_notify_destroyed();
}
