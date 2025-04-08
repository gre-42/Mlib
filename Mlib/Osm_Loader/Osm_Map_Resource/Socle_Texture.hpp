#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;
enum class PhysicsMaterial: uint32_t;

struct SocleTexture {
    std::vector<VariableAndHash<std::string>> textures;
    PhysicsMaterial material;
};

SocleTexture parse_socle_texture(const JsonMacroArguments& args);

}
