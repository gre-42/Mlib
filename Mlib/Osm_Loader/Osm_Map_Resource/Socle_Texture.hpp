#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;

struct SocleTexture {
    std::vector<VariableAndHash<std::string>> textures;
};

SocleTexture parse_socle_texture(const JsonMacroArguments& args);

}
