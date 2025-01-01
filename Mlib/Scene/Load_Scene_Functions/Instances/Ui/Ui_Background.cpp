#include "Ui_Background.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Delay_Load_Policy.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
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
    auto& bg = object_pool.create<FillPixelRegionWithTextureLogic>(
        CURRENT_SOURCE_LOCATION,
        std::make_shared<FillWithTextureLogic>(
            RenderingContextStack::primary_rendering_resources().get_texture_lazy(
                ColormapWithModifiers{
                    .filename = VariableAndHash{args.arguments.path_or_variable(KnownArgs::texture).path},
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS
                }.compute_hash(),
                TextureRole::COLOR_FROM_DB)),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        delay_load_policy_from_string(args.arguments.at<std::string>(KnownArgs::delay_load_policy)),
        FocusFilter{ .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)) });
    render_logics.append(
        { bg, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order),
        CURRENT_SOURCE_LOCATION);
}
