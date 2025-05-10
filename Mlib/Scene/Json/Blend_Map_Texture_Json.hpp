#pragma once
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <string_view>

namespace Mlib {

class JsonMacroArguments;

BlendMapTexture blend_map_texture_from_json(const JsonMacroArguments& j);
std::vector<BlendMapTexture> blend_map_textures_from_json(
    const JsonMacroArguments& j,
    std::string_view name);
std::vector<BlendMapTexture> try_blend_map_textures_from_json(
    const JsonMacroArguments& j,
    std::string_view name);

}
