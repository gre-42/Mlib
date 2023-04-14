#include "Add_Texture_Atlas.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

const std::string AddTextureAtlas::key = "add_texture_atlas";

LoadSceneUserFunction AddTextureAtlas::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)"
        "\\s+width=(\\d+)"
        "\\s+height=(\\d+)"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "\\s+images=([\\s\\S]*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void AddTextureAtlas::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::vector<AtlasTileDescriptor> tiles;
    static const DECLARE_REGEX(
        atlas_tile_reg,
        "(?:\\s*texture_pos:\\s*(\\d+)\\s+(\\d+)"
        "\\s+texture:(#?[\\w+-.\\(\\)/]+)|"
        "([\\s\\S]+))");
    find_all(match[5].str(), atlas_tile_reg, [&](const Mlib::re::smatch& match2) {
        if (match2[4].matched) {
            THROW_OR_ABORT("Unknown element: \"" + match2[4].str() + '"');
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
