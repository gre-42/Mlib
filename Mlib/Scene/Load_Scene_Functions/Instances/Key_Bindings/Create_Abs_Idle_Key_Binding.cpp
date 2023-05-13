#include "Create_Abs_Idle_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(tires_z);
}

const std::string CreateAbsIdleKeyBinding::key = "abs_idle_binding";

LoadSceneJsonUserFunction CreateAbsIdleKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAbsIdleKeyBinding(args.renderable_scene()).execute(args);
};

CreateAbsIdleKeyBinding::CreateAbsIdleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAbsIdleKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto& kb = key_bindings.add_absolute_movable_idle_binding(AbsoluteMovableIdleBinding{
        .node = &scene.get_node(args.arguments.at<std::string>(KnownArgs::node)),
        .tires_z = args.arguments.at<FixedArray<float, 3>>(
            KnownArgs::tires_z,
            FixedArray<float, 3>{0.f, 0.f, 1.f})});
    players.get_player(args.arguments.at<std::string>(KnownArgs::player))
    .append_delete_externals(
        &node,
        [&kbs=key_bindings, &kb](){
            kbs.delete_absolute_movable_idle_binding(kb);
        }
    );
}
