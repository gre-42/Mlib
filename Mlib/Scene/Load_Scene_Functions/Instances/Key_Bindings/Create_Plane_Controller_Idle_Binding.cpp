#include "Create_Plane_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

const std::string CreatePlaneControllerIdleBinding::key = "plane_controller_idle_binding";

LoadSceneUserFunction CreatePlaneControllerIdleBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(options);
    CreatePlaneControllerIdleBinding(args.renderable_scene()).execute(json_macro_arguments, args);
};

CreatePlaneControllerIdleBinding::CreatePlaneControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerIdleBinding::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    auto& n = scene.get_node(json_macro_arguments.at<std::string>(node));
    auto& kb = key_bindings.add_plane_controller_idle_binding(PlaneControllerIdleBinding{.node = &n});
    if (json_macro_arguments.contains_json(player)) {
        players.get_player(json_macro_arguments.at<std::string>(player))
        .append_delete_externals(
            &n,
            [&kbs=key_bindings, &kb](){
                kbs.delete_plane_controller_idle_binding(kb);
            }
        );
    }
}
