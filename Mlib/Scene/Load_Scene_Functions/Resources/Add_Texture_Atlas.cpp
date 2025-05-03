#include "Add_Texture_Atlas.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
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
DECLARE_ARGUMENT(mipmap_mode);
DECLARE_ARGUMENT(images);
}

namespace AtlasTileArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(source_position);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(target_position);
DECLARE_ARGUMENT(target_layer);
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
    auto& res = RenderingContextStack::primary_rendering_resources();
    auto color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode));
    auto mipmap_mode = mipmap_mode_from_string(
        args.arguments.at<std::string>(KnownArgs::mipmap_mode, "with_mipmaps"));
    auto tiles = args.arguments.children(KnownArgs::images, [&](const JsonMacroArguments& a){
        // This is not in a from_json function because of the usage of "path_or_variable".
        a.validate(AtlasTileArgs::options);
        auto source_position = a.at<UFixedArray<int, 2>>(AtlasTileArgs::source_position, fixed_zeros<int, 2>());
        auto target_position = a.at<UFixedArray<int, 2>>(AtlasTileArgs::target_position);
        auto texture_size = a.at<UFixedArray<int, 2>>(AtlasTileArgs::size, fixed_full<int, 2>(INT_MAX));
        return ManualAtlasTileDescriptor{
            .source = {
                .left = source_position(0),
                .bottom = source_position(1),
                .width = texture_size(0),
                .height = texture_size(1),
                .name = res.colormap(ColormapWithModifiers{
                    .filename = VariableAndHash{a.path_or_variable(AtlasTileArgs::texture).path},
                    .color_mode = color_mode,
                    .mipmap_mode = mipmap_mode}.compute_hash())
            },
            .target = {
                .left = target_position(0),
                .bottom = target_position(1),
                .layer = a.at<size_t>(AtlasTileArgs::target_layer, 0)
            } };
    });
    res.add_manual_texture_atlas(
        args.arguments.at(KnownArgs::name),
        ManualTextureAtlasDescriptor{
            .width = args.arguments.at<int>(KnownArgs::width),
            .height = args.arguments.at<int>(KnownArgs::height),
            .nlayers = args.arguments.at<size_t>(KnownArgs::layers, 1),
            .depth_interpolation = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::depth_interpolation, "nearest")),
            .color_mode = color_mode,
            .tiles = tiles});
}
