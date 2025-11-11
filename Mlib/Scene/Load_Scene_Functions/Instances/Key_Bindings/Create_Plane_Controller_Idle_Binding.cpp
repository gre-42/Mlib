#include "Create_Plane_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
}

const std::string CreatePlaneControllerIdleBinding::key = "plane_controller_idle_binding";

LoadSceneJsonUserFunction CreatePlaneControllerIdleBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlaneControllerIdleBinding(args.renderable_scene()).execute(args);
};

CreatePlaneControllerIdleBinding::CreatePlaneControllerIdleBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerIdleBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_plane_controller_idle_binding(std::unique_ptr<PlaneControllerIdleBinding>(new PlaneControllerIdleBinding{
        .player = player,
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } } }));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs = key_bindings, &kb]() {
            kbs.delete_plane_controller_idle_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}
