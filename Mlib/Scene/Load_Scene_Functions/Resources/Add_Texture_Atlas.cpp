#include "Add_Texture_Atlas.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(height);
DECLARE_ARGUMENT(layers);
DECLARE_ARGUMENT(depth_interpolation);
DECLARE_ARGUMENT(color_mode);
DECLARE_ARGUMENT(images);
}

namespace AtlasTileArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(layer);
DECLARE_ARGUMENT(texture);
}

const std::string AddTextureAtlas::key = "add_texture_atlas";

LoadSceneJsonUserFunction AddTextureAtlas::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddTextureAtlas::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto tiles = args.arguments.children(KnownArgs::images, [](const JsonMacroArguments& a){
        a.validate(AtlasTileArgs::options);
        auto texture_pos = a.at<FixedArray<int, 2>>(AtlasTileArgs::position);
        return ManualAtlasTileDescriptor{
            .left = texture_pos(0),
            .bottom = texture_pos(1),
            .layer = a.at<size_t>(AtlasTileArgs::layer, 0),
            .filename = a.path_or_variable(AtlasTileArgs::texture).path};
    });
    RenderingContextStack::primary_rendering_resources().add_manual_texture_atlas(
        args.arguments.at(KnownArgs::name),
        ManualTextureAtlasDescriptor{
            .width = args.arguments.at<int>(KnownArgs::width),
            .height = args.arguments.at<int>(KnownArgs::height),
            .nlayers = args.arguments.at<size_t>(KnownArgs::layers, 1),
            .depth_interpolation = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::depth_interpolation, "nearest")),
            .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
            .tiles = tiles});
}
