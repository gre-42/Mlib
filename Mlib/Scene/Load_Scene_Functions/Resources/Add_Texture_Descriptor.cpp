#include "Add_Texture_Descriptor.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

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
DECLARE_ARGUMENT(average_normal);
DECLARE_ARGUMENT(mean_color);
DECLARE_ARGUMENT(lighten);
DECLARE_ARGUMENT(lighten_left);
DECLARE_ARGUMENT(lighten_right);
DECLARE_ARGUMENT(lighten_top);
DECLARE_ARGUMENT(lighten_bottom);
DECLARE_ARGUMENT(mipmap_mode);
DECLARE_ARGUMENT(anisotropic_filtering_level);
DECLARE_ARGUMENT(wrap_mode_s);
DECLARE_ARGUMENT(wrap_mode_t);
}

const std::string AddTextureDescriptor::key = "add_texture_descriptor";

LoadSceneJsonUserFunction AddTextureDescriptor::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddTextureDescriptor::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources()->add_texture_descriptor(
        args.arguments.at<std::string>(KnownArgs::name),
        TextureDescriptor{
            .color = {
                .filename = args.arguments.path_or_variable(KnownArgs::color).path,
                .desaturate = args.arguments.at<bool>(KnownArgs::desaturate, false),
                .alpha = args.arguments.try_path_or_variable(KnownArgs::alpha).path,
                .histogram = args.arguments.try_path_or_variable(KnownArgs::histogram).path,
                .average = "",
                .multiply = args.arguments.try_path_or_variable(KnownArgs::multiply_color).path,
                .mean_color = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::mean_color, OrderableFixedArray<float, 3>(-1.f)),
                .lighten = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::lighten, OrderableFixedArray<float, 3>(0.f)),
                .lighten_left = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::lighten_left, OrderableFixedArray<float, 3>(0.f)),
                .lighten_right = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::lighten_right, OrderableFixedArray<float, 3>(0.f)),
                .lighten_top = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::lighten_top, OrderableFixedArray<float, 3>(0.f)),
                .lighten_bottom = args.arguments.at<OrderableFixedArray<float, 3>>(KnownArgs::lighten_bottom, OrderableFixedArray<float, 3>(0.f))},
            .specular = args.arguments.try_path_or_variable(KnownArgs::specular).path,
            .normal = {
                .filename = args.arguments.try_path_or_variable(KnownArgs::normal).path,
                .average =  args.arguments.try_path_or_variable(KnownArgs::average_normal).path},
            .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
            .alpha_fac = args.arguments.at<float>(KnownArgs::alpha_fac, 1.f),
            .mipmap_mode = args.arguments.contains(KnownArgs::mipmap_mode)
                ? mipmap_mode_from_string(args.arguments.at<std::string>(KnownArgs::mipmap_mode))
                : MipmapMode::WITH_MIPMAPS,
            .anisotropic_filtering_level = args.arguments.at<unsigned int>(KnownArgs::anisotropic_filtering_level),
            .wrap_modes = {
                wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_s, "repeat")),
                wrap_mode_from_string(args.arguments.at<std::string>(KnownArgs::wrap_mode_t, "repeat"))}});

}
