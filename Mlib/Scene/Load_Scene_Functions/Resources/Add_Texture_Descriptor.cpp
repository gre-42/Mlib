#include "Add_Texture_Descriptor.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction AddTextureDescriptor::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_texture_descriptor"
        "\\s+name=([\\w+-.]+)"
        "\\s+color=(#?[\\w-. \\(\\)/+-]+)"
        "(?:\\s+normal=([#\\w-. \\(\\)/+-]+))?"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "(?:\\s+desaturate=(0|1))?"
        "(?:\\s+histogram=([#\\w-. \\(\\)/+-]+))?"
        "(?:\\s+mixed=([#\\w-. \\(\\)/+-]+))?"
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
        match[1].str(),
        TextureDescriptor{
            .color = args.fpath(match[2].str()).path,
            .normal = args.fpath(match[3].str()).path,
            .color_mode = color_mode_from_string(match[4].str()),
            .desaturate = match[5].matched ? safe_stob(match[5].str()) : false,
            .histogram = args.fpath(match[6].str()).path,
            .mixed = match[7].str(),
            .overlap_npixels = match[8].matched ? safe_stoz(match[8].str()) : 0,
            .mean_color =
                OrderableFixedArray<float, 3>{
                    match[9].matched ? safe_stof(match[9].str()) : -1.f,
                    match[10].matched ? safe_stof(match[10].str()) : -1.f,
                    match[11].matched ? safe_stof(match[11].str()) : -1.f},
            .lighten =
                OrderableFixedArray<float, 3>{
                    match[12].matched ? safe_stof(match[12].str()) : 0.f,
                    match[13].matched ? safe_stof(match[13].str()) : 0.f,
                    match[14].matched ? safe_stof(match[14].str()) : 0.f},
            .anisotropic_filtering_level = safe_stou(match[15].str())});

}
