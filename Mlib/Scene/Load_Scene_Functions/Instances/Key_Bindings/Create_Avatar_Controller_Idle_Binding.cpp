#include "Create_Avatar_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

const std::string CreateAvatarControllerIdleBinding::key = "avatar_controller_idle_binding";

LoadSceneJsonUserFunction CreateAvatarControllerIdleBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAvatarControllerIdleBinding(args.renderable_scene()).execute(args);
};

CreateAvatarControllerIdleBinding::CreateAvatarControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerIdleBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_bindings.add_avatar_controller_idle_binding(AvatarControllerIdleBinding{
        .node = &scene.get_node(args.arguments.at<std::string>(KnownArgs::node))});
}
