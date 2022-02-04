#include "Burn_In.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction BurnIn::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*burn_in seconds=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        BurnIn(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

BurnIn::BurnIn(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void BurnIn::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    physics_engine.burn_in(safe_stof(match[1].str()));
    scene.move(0.f); // dt
}
