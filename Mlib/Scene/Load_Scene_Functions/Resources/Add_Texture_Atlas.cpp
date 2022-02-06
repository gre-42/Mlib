#include "Add_Texture_Atlas.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>

using namespace Mlib;

LoadSceneUserFunction AddTextureAtlas::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_texture_atlas"
        "\\s+name=([\\w+-.]+)"
        "\\s+width=(\\d+)"
        "\\s+height=(\\d+)"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "\\s+images=([\\s\\S]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddTextureAtlas::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::vector<AtlasTileDescriptor> tiles;
    static const DECLARE_REGEX(
        atlas_tile_reg,
        "(?:\\s*texture_pos:\\s*(\\d+)\\s+(\\d+)"
        "\\s+texture:(#?[\\w-.\\(\\)/+-]+)|"
        "([\\s\\S]+))");
    find_all(match[5].str(), atlas_tile_reg, [&](const Mlib::re::smatch& match2) {
        if (match2[4].matched) {
            throw std::runtime_error("Unknown element: \"" + match2[4].str() + '"');
        }
        tiles.push_back(AtlasTileDescriptor{
            .left = safe_stoi(match2[1].str()),
            .bottom = safe_stoi(match2[2].str()),
            .filename = args.fpath(match2[3].str()).path});
    });
    RenderingContextStack::primary_rendering_resources()->add_texture_atlas(
        match[1].str(),
        TextureAtlasDescriptor{
            .width = safe_stoi(match[2].str()),
            .height = safe_stoi(match[3].str()),
            .color_mode = color_mode_from_string(match[4].str()),
            .tiles = tiles});
}
