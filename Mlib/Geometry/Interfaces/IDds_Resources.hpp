#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Mlib {

enum class TextureAlreadyExistsBehavior {
    WARN,
    RAISE
};

struct ColormapWithModifiers;

class IDdsResources {
public:
    virtual void add_texture(
        const ColormapWithModifiers& name,
        std::vector<uint8_t>&& data,
        TextureAlreadyExistsBehavior already_exists_behavior) = 0;
};

}
