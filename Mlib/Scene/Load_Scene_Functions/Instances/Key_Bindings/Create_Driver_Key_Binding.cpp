#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(select_next_opponent);
DECLARE_ARGUMENT(select_next_vehicle);
}

const std::string CreateDriverKeyBinding::key = "player_key_binding";

LoadSceneJsonUserFunction CreateDriverKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDriverKeyBinding(args.renderable_scene()).execute(args);
};

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    if (rb->driver_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no driver");
    }
    auto player = dynamic_cast<Player*>(rb->driver_);
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not player");
    }
    auto& kb = key_bindings.add_player_key_binding(PlayerKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .node = node.ptr(),
        .select_next_opponent = args.arguments.at<bool>(KnownArgs::select_next_opponent, false),
        .select_next_vehicle = args.arguments.at<bool>(KnownArgs::select_next_vehicle, false)});
    player->append_delete_externals(
        node.ptr(),
        [&kbs=key_bindings, &kb](){
            kbs.delete_player_key_binding(kb);
        }
    );
}
