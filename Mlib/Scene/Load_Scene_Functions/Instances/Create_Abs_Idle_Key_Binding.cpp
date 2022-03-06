#include "Create_Abs_Idle_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(TIRES_Z_X);
DECLARE_OPTION(TIRES_Z_Y);
DECLARE_OPTION(TIRES_Z_Z);

LoadSceneUserFunction CreateAbsIdleKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*abs_idle_binding"
        "\\s+node=([\\w+-.]+)"
        "(?:\\s+tires_z=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateAbsIdleKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateAbsIdleKeyBinding::CreateAbsIdleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAbsIdleKeyBinding::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_absolute_movable_idle_binding(AbsoluteMovableIdleBinding{
        .node = &scene.get_node(match[NODE].str()),
        .tires_z = {
            match[TIRES_Z_X].str().empty() ? 0.f : safe_stof(match[TIRES_Z_X].str()),
            match[TIRES_Z_Y].str().empty() ? 0.f : safe_stof(match[TIRES_Z_Y].str()),
            match[TIRES_Z_Z].str().empty() ? 1.f : safe_stof(match[TIRES_Z_Z].str())}});
}
