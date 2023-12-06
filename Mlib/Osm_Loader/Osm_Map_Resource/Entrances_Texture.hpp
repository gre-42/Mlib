#pragma once
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments;

struct EntrancesTexture {
    std::vector<std::string> textures;
    float uv_scale_x;
};

EntrancesTexture parse_entrances_texture(const JsonMacroArguments& args);

}
