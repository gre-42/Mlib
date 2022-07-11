#include "Create_Plane_Controller_Idle_Binding.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

LoadSceneUserFunction CreatePlaneControllerIdleBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*plane_controller_idle_binding"
        "(?:\\s+player=([\\w+-.]+))?"
        "\\s+node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreatePlaneControllerIdleBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreatePlaneControllerIdleBinding::CreatePlaneControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerIdleBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto& kb = key_bindings.add_plane_controller_idle_binding(PlaneControllerIdleBinding{.node = &node});
    if (match[PLAYER].matched) {
        players.get_player(match[PLAYER].str())
        .append_delete_externals(
            &node,
            [&kbs=key_bindings, &kb](){
                kbs.delete_plane_controller_idle_binding(kb);
            }
        );
    }
}
