#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;
enum class PhysicsMaterial: uint32_t;
class FPath;

struct SocleTexture {
    std::vector<FPath> textures;
    PhysicsMaterial material;
};

SocleTexture parse_socle_texture(const JsonMacroArguments& args);

}
