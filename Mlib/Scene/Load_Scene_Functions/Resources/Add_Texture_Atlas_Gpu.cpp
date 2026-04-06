#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <stdexcept>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(height);
DECLARE_ARGUMENT(layers);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(depth_interpolation);
DECLARE_ARGUMENT(color_mode);
DECLARE_ARGUMENT(mipmap_mode);
DECLARE_ARGUMENT(wrap_mode_s);
DECLARE_ARGUMENT(wrap_mode_t);
DECLARE_ARGUMENT(anisotropic_filtering_level);
DECLARE_ARGUMENT(mip_level_count);
DECLARE_ARGUMENT(images);
}

namespace AtlasTileArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(height);
DECLARE_ARGUMENT(texture);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_texture_atlas_gpu",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto& res = RenderingContextStack::primary_rendering_resources();
                auto color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode));
                auto mipmap_mode = mipmap_mode_from_string(
                    args.arguments.at<std::string>(KnownArgs::mipmap_mode, "with_mipmaps"));
                auto tiles = args.arguments.at_vector<nlohmann::json>(KnownArgs::images, [&](const nlohmann::json& layer){
                    JsonView layer_view{layer, CheckIsObjectBehavior::NO_CHECK};
                    return layer_view.get_vector<nlohmann::json>([&](const nlohmann::json& j){
                        auto a = args.arguments.as_child(j);
                        a.validate(AtlasTileArgs::options);
                        return AutoAtlasTileDescriptor{
                            .left = a.at<int>(AtlasTileArgs::left),
                            .bottom = a.at<int>(AtlasTileArgs::bottom),
                            .width = a.at<int>(AtlasTileArgs::width),
                            .height = a.at<int>(AtlasTileArgs::height),
                            .name = res.colormap(ColormapWithModifiers{
                                .filename = a.path_or_variable(AtlasTileArgs::texture),
                                .color_mode = color_mode,
                                .mipmap_mode = mipmap_mode}.compute_hash())
                            };
                    });
                });
                res.add_auto_texture_atlas(
                    ColormapWithModifiers{
                        .filename = FPath::from_variable(args.arguments.at(KnownArgs::name)),
                        .color_mode = color_mode,
                        .mipmap_mode = mipmap_mode,
                        .magnifying_interpolation_mode = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::magnifying_interpolation_mode, "linear")),
                        .depth_interpolation = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::depth_interpolation, "nearest")),
                        .anisotropic_filtering_level = args.arguments.at<unsigned int>(KnownArgs::anisotropic_filtering_level),
                        .wrap_modes = {
                            wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_s, "repeat")),
                            wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_t, "repeat"))},
                    }.compute_hash(),
                    AutoTextureAtlasDescriptor{
                        .width = args.arguments.at<int>(KnownArgs::width),
                        .height = args.arguments.at<int>(KnownArgs::height),
                        .mip_level_count = args.arguments.at<int>(KnownArgs::mip_level_count),
                        .tiles = tiles});
            });
    }
} obj;

}
