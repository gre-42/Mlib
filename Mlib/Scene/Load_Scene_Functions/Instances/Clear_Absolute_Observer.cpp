#include "Clear_Absolute_Observer.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction ClearAbsoluteObserver::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*clear_absolute_observer"
        "\\s+node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ClearAbsoluteObserver(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ClearAbsoluteObserver::ClearAbsoluteObserver(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearAbsoluteObserver::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.get_node(match[1].str()).clear_absolute_observer();
}
