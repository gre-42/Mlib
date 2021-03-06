#include "Add_Texture_Descriptor.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(COLOR);
DECLARE_OPTION(ALPHA);
DECLARE_OPTION(SPECULAR);
DECLARE_OPTION(NORMAL);
DECLARE_OPTION(COLOR_MODE);
DECLARE_OPTION(DESATURATE);
DECLARE_OPTION(HISTOGRAM);
DECLARE_OPTION(MIXED);
DECLARE_OPTION(OVERLAP_NPIXELS);
DECLARE_OPTION(MEAN_COLOR_R);
DECLARE_OPTION(MEAN_COLOR_G);
DECLARE_OPTION(MEAN_COLOR_B);
DECLARE_OPTION(LIGHTEN_R);
DECLARE_OPTION(LIGHTEN_G);
DECLARE_OPTION(LIGHTEN_B);
DECLARE_OPTION(ANISOTROPIC_FILTERING_LEVEL);

LoadSceneUserFunction AddTextureDescriptor::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_texture_descriptor"
        "\\s+name=([\\w+-.]+)"
        "\\s+color=(#?[\\w+-. \\(\\)/]+)"
        "(?:\\s+alpha=([#\\w+-. \\(\\)/]+))?"
        "(?:\\s+specular=([#\\w+-. \\(\\)/]+))?"
        "(?:\\s+normal=([#\\w+-. \\(\\)/]+))?"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "(?:\\s+desaturate=(0|1))?"
        "(?:\\s+histogram=([#\\w+-. \\(\\)/]+))?"
        "(?:\\s+mixed=([#\\w+-. \\(\\)/]+))?"
        "(?:\\s+overlap_npixels=(\\d+))?"
        "(?:\\s+mean_color=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+lighten=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+anisotropic_filtering_level=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddTextureDescriptor::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources()->add_texture_descriptor(
        match[NAME].str(),
        TextureDescriptor{
            .color = args.fpath(match[COLOR].str()).path,
            .alpha = args.fpath(match[ALPHA].str()).path,
            .specular = args.fpath(match[SPECULAR].str()).path,
            .normal = args.fpath(match[NORMAL].str()).path,
            .color_mode = color_mode_from_string(match[COLOR_MODE].str()),
            .desaturate = match[DESATURATE].matched ? safe_stob(match[DESATURATE].str()) : false,
            .histogram = args.fpath(match[HISTOGRAM].str()).path,
            .mixed = match[MIXED].str(),
            .overlap_npixels = match[OVERLAP_NPIXELS].matched ? safe_stoz(match[OVERLAP_NPIXELS].str()) : 0,
            .mean_color =
                OrderableFixedArray<float, 3>{
                    match[MEAN_COLOR_R].matched ? safe_stof(match[MEAN_COLOR_R].str()) : -1.f,
                    match[MEAN_COLOR_G].matched ? safe_stof(match[MEAN_COLOR_G].str()) : -1.f,
                    match[MEAN_COLOR_B].matched ? safe_stof(match[MEAN_COLOR_B].str()) : -1.f},
            .lighten =
                OrderableFixedArray<float, 3>{
                    match[LIGHTEN_R].matched ? safe_stof(match[LIGHTEN_R].str()) : 0.f,
                    match[LIGHTEN_G].matched ? safe_stof(match[LIGHTEN_G].str()) : 0.f,
                    match[LIGHTEN_B].matched ? safe_stof(match[LIGHTEN_B].str()) : 0.f},
            .anisotropic_filtering_level = safe_stou(match[ANISOTROPIC_FILTERING_LEVEL].str())});

}
