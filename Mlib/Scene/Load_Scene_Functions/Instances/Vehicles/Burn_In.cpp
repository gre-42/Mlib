#include "Burn_In.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SECONDS);

LoadSceneUserFunction BurnIn::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*burn_in seconds=([\\w+-.]+)$");
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    physics_engine.burn_in(safe_stof(match[SECONDS].str()) * s);
    scene.move(0.f); // dt
}
