#pragma once
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;

struct SocleTexture {
    std::vector<std::string> textures;
};

SocleTexture parse_socle_texture(const JsonMacroArguments& args);

}
