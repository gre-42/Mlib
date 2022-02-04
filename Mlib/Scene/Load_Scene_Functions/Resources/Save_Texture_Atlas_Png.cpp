#include "Save_Texture_Atlas_Png.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

LoadSceneUserFunction SaveTextureAtlasPng::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*save_texture_atlas_png"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+color_mode=(grayscale|rgb|rgba)");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void SaveTextureAtlasPng::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    StbInfo img = RenderingContextStack::primary_rendering_resources()->get_texture_data(TextureDescriptor{
        .color = match[1].str(),
        .color_mode = color_mode_from_string(match[3].str())});
    if (!stbi_write_png(
        match[2].str().c_str(),
        img.width,
        img.height,
        img.nrChannels,
        img.data.get(),
        0))
    {
        throw std::runtime_error("Could not write \"" + match[2].str() + '"');
    }
}
