#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(alpha);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(normal);
DECLARE_ARGUMENT(color_mode);
DECLARE_ARGUMENT(alpha_fac);
DECLARE_ARGUMENT(desaturate);
DECLARE_ARGUMENT(histogram);
DECLARE_ARGUMENT(multiply_color);
DECLARE_ARGUMENT(alpha_blend);
DECLARE_ARGUMENT(average_normal);
DECLARE_ARGUMENT(mean_color);
DECLARE_ARGUMENT(lighten);
DECLARE_ARGUMENT(lighten_left);
DECLARE_ARGUMENT(lighten_right);
DECLARE_ARGUMENT(lighten_top);
DECLARE_ARGUMENT(lighten_bottom);
DECLARE_ARGUMENT(color_to_replace);
DECLARE_ARGUMENT(replacement_color);
DECLARE_ARGUMENT(replacement_tolerance);
DECLARE_ARGUMENT(selected_color);
DECLARE_ARGUMENT(selected_color_near);
DECLARE_ARGUMENT(selected_color_far);
DECLARE_ARGUMENT(edge_sigma);
DECLARE_ARGUMENT(times);
DECLARE_ARGUMENT(plus);
DECLARE_ARGUMENT(abs);
DECLARE_ARGUMENT(invert);
DECLARE_ARGUMENT(height_to_normals);
DECLARE_ARGUMENT(saturate);
DECLARE_ARGUMENT(multiply_with_alpha);
DECLARE_ARGUMENT(mipmap_mode);
DECLARE_ARGUMENT(depth_interpolation);
DECLARE_ARGUMENT(anisotropic_filtering_level);
DECLARE_ARGUMENT(wrap_mode_s);
DECLARE_ARGUMENT(wrap_mode_t);
DECLARE_ARGUMENT(rotate);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_texture_descriptor",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto mipmap_mode = mipmap_mode_from_string(
        args.arguments.at<std::string>(KnownArgs::mipmap_mode, "with_mipmaps"));
    auto anisotropic_filtering_level = args.arguments.at<unsigned int>(KnownArgs::anisotropic_filtering_level);
    auto wrap_modes = OrderableFixedArray<WrapMode, 2>{
        wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_s, "repeat")),
        wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_t, "repeat"))};
    auto rotate = args.arguments.at<int>(KnownArgs::rotate, 0);
    auto normal = ColormapWithModifiers{
        .filename = VariableAndHash{args.arguments.try_path_or_variable(KnownArgs::normal).path},
        .average = args.arguments.try_path_or_variable(KnownArgs::average_normal).path,
        .color_mode = ColorMode::RGB,
        .mipmap_mode = mipmap_mode,
        .anisotropic_filtering_level = anisotropic_filtering_level,
        .wrap_modes = wrap_modes,
        .rotate = rotate}.compute_hash();
    auto specular = ColormapWithModifiers{
        .filename = VariableAndHash{args.arguments.try_path_or_variable(KnownArgs::specular).path},
        .color_mode = ColorMode::RGB,
        .mipmap_mode = mipmap_mode,
        .anisotropic_filtering_level = anisotropic_filtering_level,
        .wrap_modes = wrap_modes,
        .rotate = rotate}.compute_hash();
    {
        auto filename = args.arguments.try_path_or_variable(KnownArgs::normal);
        if (filename.is_variable) {
            normal = RenderingContextStack::primary_rendering_resources().colormap(normal);
        }
    }
    {
        auto filename = args.arguments.try_path_or_variable(KnownArgs::specular);
        if (filename.is_variable) {
            specular = RenderingContextStack::primary_rendering_resources().colormap(specular);
        }
    }
    RenderingContextStack::primary_rendering_resources().add_texture_descriptor(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        TextureDescriptor{
            .color = ColormapWithModifiers{
                .filename = VariableAndHash{args.arguments.path_or_variable(KnownArgs::color).path},
                .desaturate = args.arguments.at<float>(KnownArgs::desaturate, 0.f),
                .alpha = args.arguments.try_path_or_variable(KnownArgs::alpha).path,
                .histogram = args.arguments.try_path_or_variable(KnownArgs::histogram).path,
                .average = "",
                .multiply = args.arguments.try_path_or_variable(KnownArgs::multiply_color).path,
                .alpha_blend = args.arguments.try_path_or_variable(KnownArgs::alpha_blend).path,
                .mean_color = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::mean_color, OrderableFixedArray<float, 3>(-1.f)),
                .lighten = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::lighten, OrderableFixedArray<float, 3>(0.f)),
                .lighten_left = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::lighten_left, OrderableFixedArray<float, 3>(0.f)),
                .lighten_right = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::lighten_right, OrderableFixedArray<float, 3>(0.f)),
                .lighten_top = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::lighten_top, OrderableFixedArray<float, 3>(0.f)),
                .lighten_bottom = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::lighten_bottom, OrderableFixedArray<float, 3>(0.f)),
                .color_to_replace = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::color_to_replace, OrderableFixedArray<float, 3>(-1.f)),
                .replacement_color = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::replacement_color, OrderableFixedArray<float, 3>(-1.f)),
                .replacement_tolerance = args.arguments.at<float>(KnownArgs::replacement_tolerance, 0.f),
                .selected_color = args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::selected_color, OrderableFixedArray<float, 3>(0.f)),
                .selected_color_near = args.arguments.at<float>(KnownArgs::selected_color_near, 0),
                .selected_color_far = args.arguments.at<float>(KnownArgs::selected_color_far, INFINITY),
                .edge_sigma = args.arguments.at<float>(KnownArgs::edge_sigma, 0.f),
                .times = args.arguments.at<float>(KnownArgs::times, 1.f),
                .plus = args.arguments.at<float>(KnownArgs::plus, 0.f),
                .abs = args.arguments.at<bool>(KnownArgs::abs, false),
                .invert = args.arguments.at<bool>(KnownArgs::invert, false),
                .height_to_normals = args.arguments.at<bool>(KnownArgs::height_to_normals, false),
                .saturate = args.arguments.at<bool>(KnownArgs::saturate, false),
                .multiply_with_alpha = args.arguments.at<bool>(KnownArgs::multiply_with_alpha, false),
                .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
                .alpha_fac = args.arguments.at<float>(KnownArgs::alpha_fac, 1.f),
                .mipmap_mode = mipmap_mode,
                .depth_interpolation = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::depth_interpolation, "nearest")),
                .anisotropic_filtering_level = anisotropic_filtering_level,
                .wrap_modes = wrap_modes,
                .rotate = rotate}.compute_hash(),
            .specular = specular,
            .normal = normal });
            });
    }
} obj;

}
