#include "Save_Texture_Atlas_Png.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(color_mode);
}

const std::string SaveTextureAtlasPng::key = "save_texture_atlas_png";

LoadSceneJsonUserFunction SaveTextureAtlasPng::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void SaveTextureAtlasPng::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string filename = args.arguments.at<std::string>(KnownArgs::filename);
    if (!filename.ends_with(".png")) {
        THROW_OR_ABORT("Filename \"" + filename + "\" does not end with .png");
    }
    StbInfo img = RenderingContextStack::primary_rendering_resources()->get_texture_data(TextureDescriptor{
        .color = args.arguments.at<std::string>(KnownArgs::name),
        .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode))});
    if (!stbi_write_png(
        filename.c_str(),
        img.width,
        img.height,
        img.nrChannels,
        img.data.get(),
        0))
    {
        THROW_OR_ABORT("Could not write \"" + filename + '"');
    }
}
