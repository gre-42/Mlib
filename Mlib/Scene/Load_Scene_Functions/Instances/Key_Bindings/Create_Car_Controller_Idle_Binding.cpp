#include "Create_Car_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(surface_power);
DECLARE_ARGUMENT(steer_angle);
DECLARE_ARGUMENT(drive_relaxation);
DECLARE_ARGUMENT(steer_relaxation);
}

const std::string CreateCarControllerIdleBinding::key = "car_controller_idle_binding";

LoadSceneUserFunction CreateCarControllerIdleBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(KnownArgs::options);
    CreateCarControllerIdleBinding(args.renderable_scene()).execute(json_macro_arguments, args);
};

CreateCarControllerIdleBinding::CreateCarControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCarControllerIdleBinding::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    auto& n = scene.get_node(json_macro_arguments.at<std::string>(KnownArgs::node));
    auto& kb = key_bindings.add_car_controller_idle_binding(CarControllerIdleBinding{
        .node = &n,
        .surface_power = json_macro_arguments.at<float>(KnownArgs::surface_power, 0.f) * W,
        .steer_angle = json_macro_arguments.at<float>(KnownArgs::steer_angle, 0.f) * degrees,
        .drive_relaxation = json_macro_arguments.at<float>(KnownArgs::drive_relaxation, 0.f),
        .steer_relaxation = json_macro_arguments.at<float>(KnownArgs::steer_relaxation, 0.f)});
    if (json_macro_arguments.contains_json(KnownArgs::player)) {
        players.get_player(json_macro_arguments.at<std::string>(KnownArgs::player))
        .append_delete_externals(
            &n,
            [&kbs=key_bindings, &kb](){
                kbs.delete_car_controller_idle_binding(kb);
            }
        );
    }
}
