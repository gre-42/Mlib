#include "Save_Texture_Atlas_Png.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(COLOR_MODE);

const std::string SaveTextureAtlasPng::key = "save_texture_atlas_png";

LoadSceneUserFunction SaveTextureAtlasPng::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+color_mode=(grayscale|rgb|rgba)");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void SaveTextureAtlasPng::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string filename = match[FILENAME].str();
    if (!filename.ends_with(".png")) {
        THROW_OR_ABORT("Filename \"" + filename + "\" does not end with .png");
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
        THROW_OR_ABORT("Could not write \"" + filename + '"');
    }
}
