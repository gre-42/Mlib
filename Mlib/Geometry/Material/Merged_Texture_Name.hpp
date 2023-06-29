#pragma once
#include <string>

namespace Mlib {

struct Material;

struct MergedTextureName {
    explicit MergedTextureName(const Material& material);
    const std::string& name;
};

}
