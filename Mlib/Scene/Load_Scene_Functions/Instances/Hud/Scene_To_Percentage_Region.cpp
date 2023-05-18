#include "Scene_To_Percentage_Region.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Percentage_Region_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(target_scene);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

const std::string SceneToPercentageRegion::key = "scene_to_percentage_region";

LoadSceneJsonUserFunction SceneToPercentageRegion::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SceneToPercentageRegion(args.renderable_scene()).execute(args);
};

SceneToPercentageRegion::SceneToPercentageRegion(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SceneToPercentageRegion::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& rs = args.renderable_scenes[args.arguments.at<std::string>(KnownArgs::target_scene)];
    std::shared_ptr<RenderToPercentageRegionLogic> render_scene_to_pixel_region_logic_;
    render_scene_to_pixel_region_logic_ = std::make_shared<RenderToPercentageRegionLogic>(
        renderable_scene,
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::position),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::size),
        FocusFilter{
            .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
            .submenu_ids = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::submenus, {})});
    RenderingContextGuard rcg{ RenderingContext {
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .particle_resources = secondary_rendering_context.particle_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = args.arguments.at<int>(KnownArgs::z_order) }};
    rs.render_logics_.append(nullptr, render_scene_to_pixel_region_logic_);
}
