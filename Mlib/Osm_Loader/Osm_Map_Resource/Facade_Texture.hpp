#pragma once
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <cmath>
#include <string>

namespace Mlib {

struct FacadeTextureDescriptor {
    std::string name;
    InteriorTextures interior_textures;
};

struct FacadeTexture {
    std::string selector;
    float min_height = -INFINITY;
    float max_height = INFINITY;
    FacadeTextureDescriptor descriptor;
};

FacadeTexture parse_facade_texture(const std::string& name);

}
