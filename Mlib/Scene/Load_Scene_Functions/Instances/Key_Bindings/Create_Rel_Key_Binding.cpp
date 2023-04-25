#include "Create_Rel_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(rotation_axis);
DECLARE_ARGUMENT(angular_velocity_press);
DECLARE_ARGUMENT(angular_velocity_repeat);
DECLARE_ARGUMENT(speed_cursor);
}

const std::string CreateRelKeyBinding::key = "rel_key_binding";

LoadSceneJsonUserFunction CreateRelKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRelKeyBinding(args.renderable_scene()).execute(args);
};

CreateRelKeyBinding::CreateRelKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_bindings.add_relative_movable_key_binding(RelativeMovableKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .node = &scene.get_node(args.arguments.at<std::string>(KnownArgs::node)),
        .rotation_axis = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation_axis),
        .angular_velocity_press = args.arguments.at<float>(KnownArgs::angular_velocity_press) * radians / s,
        .angular_velocity_repeat = args.arguments.at<float>(KnownArgs::angular_velocity_repeat) * radians / s,
        .speed_cursor = args.arguments.at<float>(KnownArgs::speed_cursor)});
}
