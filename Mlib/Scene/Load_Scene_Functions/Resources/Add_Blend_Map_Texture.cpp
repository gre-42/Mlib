#include "Add_Blend_Map_Texture.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction AddBlendMapTexture::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_blend_map_texture"
        "\\s+name=([\\w-. \\(\\)/+-]+)"
        "\\s+texture=(#?[\\w-.\\(\\)/+-]+)"
        "\\s+min_height=([\\w+-.]+)"
        "\\s+max_height=([\\w+-.]+)"
        "\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+normal=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cosine=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+discreteness=([\\w+-.]+))?"
        "\\s+scale=([\\w+-.]+)"
        "\\s+weight=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddBlendMapTexture::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rr = RenderingContextStack::primary_rendering_resources();
    auto tex = args.fpath(match[2].str());
    rr->set_blend_map_texture(
        match[1].str(),
        BlendMapTexture{
            .texture_descriptor = tex.is_variable
                ? RenderingContextStack::primary_rendering_resources()->get_existing_texture_descriptor(tex.path)
                : TextureDescriptor{ .color = tex.path },
            .min_height = safe_stof(match[3].str()),
            .max_height = safe_stof(match[4].str()),
            .distances = {
                safe_stof(match[5].str()),
                safe_stof(match[6].str()),
                safe_stof(match[7].str()),
                safe_stof(match[8].str())},
            .normal = {
                safe_stof(match[9].str()),
                safe_stof(match[10].str()),
                safe_stof(match[11].str())},
            .cosines = {
                safe_stof(match[12].str()),
                safe_stof(match[13].str()),
                safe_stof(match[14].str()),
                safe_stof(match[15].str())},
            .discreteness = match[16].matched ? safe_stof(match[16].str()) : 2,
            .scale = safe_stof(match[17].str()),
            .weight = safe_stof(match[18].str()) });
}
