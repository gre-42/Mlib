#include "Save_Texture_Atlas_Png.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(COLOR_MODE);

LoadSceneUserFunction SaveTextureAtlasPng::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*save_texture_atlas_png"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+color_mode=(grayscale|rgb|rgba)");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void SaveTextureAtlasPng::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string filename = match[FILENAME].str();
    if (!filename.ends_with(".png")) {
        throw std::runtime_error("Filename \"" + filename + "\" does not end with .png");
    }
    StbInfo img = RenderingContextStack::primary_rendering_resources()->get_texture_data(TextureDescriptor{
        .color = match[NAME].str(),
        .color_mode = color_mode_from_string(match[COLOR_MODE].str())});
    if (!stbi_write_png(
        filename.c_str(),
        img.width,
        img.height,
        img.nrChannels,
        img.data.get(),
        0))
    {
        throw std::runtime_error("Could not write \"" + filename + '"');
    }
}
