#include "Create_Skidmark.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skidmark_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
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
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto& o = global_object_pool.create<SkidmarkLogic>(
        CURRENT_SOURCE_LOCATION,
        node,
        skidmark,
        *skidmark_particles.particle_renderer,
        args.arguments.at<int>(KnownArgs::texture_width),
        args.arguments.at<int>(KnownArgs::texture_height));
    o.on_skidmark_node_clear.add([&o](){ global_object_pool.remove(o); }, CURRENT_SOURCE_LOCATION);
    render_logics.prepend(
        { o, CURRENT_SOURCE_LOCATION },
        0 /* z_order */,
        CURRENT_SOURCE_LOCATION);
    node->add_skidmark(skidmark);
}
