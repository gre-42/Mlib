#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Driver.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
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
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    auto& driver = get_driver(rb);
    auto& kb = key_bindings.add_player_key_binding(PlayerKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .node = node.ptr(),
        .select_next_opponent = args.arguments.at<bool>(KnownArgs::select_next_opponent, false),
        .select_next_vehicle = args.arguments.at<bool>(KnownArgs::select_next_vehicle, false)});
    driver.append_delete_externals(
        node.ptr(),
        [&kbs=key_bindings, &kb](){
            kbs.delete_player_key_binding(kb);
        }
    );
}
