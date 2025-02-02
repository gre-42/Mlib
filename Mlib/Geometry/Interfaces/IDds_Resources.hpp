#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

enum class TextureAlreadyExistsBehavior {
    WARN,
    RAISE
};

enum class FlipMode;

struct ColormapWithModifiers;

class IDdsResources {
public:
    virtual void add_texture(
        const ColormapWithModifiers& name,
        std::vector<std::byte>&& data,
        FlipMode flip_mode,
        TextureAlreadyExistsBehavior already_exists_behavior) = 0;
};

}
