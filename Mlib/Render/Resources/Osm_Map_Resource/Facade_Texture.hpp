#pragma once
#include <cmath>
#include <string>

namespace Mlib {

struct FacadeTexture {
    std::string name;
    float min_height = -INFINITY;
    float max_height = INFINITY;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(min_height);
        archive(max_height);
    }
};

FacadeTexture parse_facade_texture(const std::string& name);

}
