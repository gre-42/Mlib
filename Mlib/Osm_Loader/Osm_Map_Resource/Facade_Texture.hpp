#pragma once
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;
enum class PhysicsMaterial: uint32_t;

struct FacadeTextureDescriptor {
    std::vector<VariableAndHash<std::string>> names;
    PhysicsMaterial material;
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
