#include "Create_Avatar_Controller_Idle_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

LoadSceneUserFunction CreateAvatarControllerIdleBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*avatar_controller_idle_binding"
        "\\s+node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateAvatarControllerIdleBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateAvatarControllerIdleBinding::CreateAvatarControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerIdleBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_avatar_controller_idle_binding(AvatarControllerIdleBinding{
        .node = &scene.get_node(match[NODE].str())});
}
