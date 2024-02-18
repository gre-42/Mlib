#include "Create_Skidmark.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skidmark_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(black_nodes);
DECLARE_ARGUMENT(texture_width);
DECLARE_ARGUMENT(texture_height);
}

const std::string CreateSkidmark::key = "create_skidmark";

LoadSceneJsonUserFunction CreateSkidmark::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateSkidmark(args.renderable_scene()).execute(args);
};

CreateSkidmark::CreateSkidmark(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateSkidmark::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::scoped_lock lock_guard{ delete_node_mutex };
    auto node_name = args.arguments.at<std::string>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto resource_suffix = "skidmark" + scene.get_temporary_instance_suffix();
    render_logics.prepend(
        node.ptr(),
        std::make_shared<SkidmarkLogic>(
            rendering_resources,
            node,
            resource_suffix,
            particle_renderer,
            args.arguments.at<int>(KnownArgs::texture_width),
            args.arguments.at<int>(KnownArgs::texture_height)),
        0 /* z_order */);
    node->add_skidmark(std::make_unique<Skidmark>(Skidmark{
        .resource_suffix = resource_suffix}));
}
