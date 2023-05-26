#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

class SceneNode;

class IImposters {
public:
    virtual void create_imposter(
        SceneNode& scene_node,
        const std::string& debug_prefix,
        uint32_t max_texture_size) = 0;
};

}
