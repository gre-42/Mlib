#include "Fill_Pixel_Region_With_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Delay_Load_Policy.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(source_scene);
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(delay_load_policy);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

const std::string FillPixelRegionWithTexture::key = "fill_pixel_region_with_texture";

LoadSceneJsonUserFunction FillPixelRegionWithTexture::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    FillPixelRegionWithTexture(args.renderable_scene()).execute(args);
};

FillPixelRegionWithTexture::FillPixelRegionWithTexture(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FillPixelRegionWithTexture::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string source_scene = args.arguments.at<std::string>(KnownArgs::source_scene);
    auto& rs = args.renderable_scenes[source_scene];
    auto& scene_window_logic = object_pool.create<FillPixelRegionWithTextureLogic>(
        CURRENT_SOURCE_LOCATION,
        std::make_shared<FillWithTextureLogic>(
            rs.rendering_resources_.get_texture_lazy(
                ColormapWithModifiers{
                    .filename = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::texture),
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::NO_MIPMAPS
                }.compute_hash()),
            CullFaceMode::CULL,
            ContinuousBlendMode::ALPHA),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        delay_load_policy_from_string(args.arguments.at<std::string>(KnownArgs::delay_load_policy)),
        FocusFilter{
            .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
            .submenu_ids = string_to_set(args.arguments.at<std::string>(KnownArgs::submenus, {}))});
    render_logics.append({ scene_window_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
}
