#include "Save_Texture_Array_Png.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename_prefix);
DECLARE_ARGUMENT(color_mode);
DECLARE_ARGUMENT(mipmap_mode);
}

const std::string SaveTextureArrayPng::key = "save_texture_array_png";

LoadSceneJsonUserFunction SaveTextureArrayPng::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void SaveTextureArrayPng::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources().save_array_to_file(
        args.arguments.at<std::string>(KnownArgs::filename_prefix),
        ColormapWithModifiers{
            .filename = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
            .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
            .mipmap_mode = mipmap_mode_from_string(
                args.arguments.at<std::string>(KnownArgs::mipmap_mode, "with_mipmaps"))}.compute_hash(),
        TextureRole::COLOR_FROM_DB);
}
