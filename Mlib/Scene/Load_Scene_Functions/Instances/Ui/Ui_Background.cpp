#include "Ui_Background.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Delay_Load_Policy.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(update);
DECLARE_ARGUMENT(delay_load_policy);
DECLARE_ARGUMENT(focus_mask);
}

const std::string UiBackground::key = "ui_background";

LoadSceneJsonUserFunction UiBackground::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    UiBackground(args.renderable_scene()).execute(args);
};

UiBackground::UiBackground(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void UiBackground::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .particle_resources = primary_rendering_context.particle_resources,    // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .z_order = args.arguments.at<int>(KnownArgs::z_order)} };                           // read by RenderLogics
    auto bg = std::make_shared<FillPixelRegionWithTextureLogic>(
        std::make_shared<FillWithTextureLogic>(
            args.arguments.path(KnownArgs::texture),
            resource_update_cycle_from_string(args.arguments.at<std::string>(KnownArgs::update)),
            ColorMode::RGBA),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        delay_load_policy_from_string(args.arguments.at<std::string>(KnownArgs::delay_load_policy)),
        FocusFilter{ .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)) });
    render_logics.append(nullptr, bg);
}
