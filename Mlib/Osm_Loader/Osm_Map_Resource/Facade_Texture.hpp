#pragma once
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <cmath>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;

struct FacadeTextureDescriptor {
    std::vector<std::string> names;
    float uv_scale_x;
    InteriorTextures interior_textures;
};

struct FacadeTexture {
    std::string selector;
    float min_height = -INFINITY;
    float max_height = INFINITY;
    FacadeTextureDescriptor descriptor;
};

FacadeTexture parse_facade_texture(const JsonMacroArguments& args);

}
