#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;

class IImposters {
public:
    virtual void create_imposter(
        DanglingRef<SceneNode> scene_node,
        const std::string& debug_prefix,
        uint32_t max_texture_size) = 0;
};

}
