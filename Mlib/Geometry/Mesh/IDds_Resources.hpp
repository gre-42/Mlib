#pragma once
#include <string>
#include <vector>

namespace Mlib {

enum class TextureAlreadyExistsBehavior {
    IGNORE,
    RAISE
};

class IDdsResources {
public:
    virtual void insert_texture(
        const std::string& name,
        std::vector<uint8_t>&& data,
        TextureAlreadyExistsBehavior already_exists_behavior) = 0;
};

}
