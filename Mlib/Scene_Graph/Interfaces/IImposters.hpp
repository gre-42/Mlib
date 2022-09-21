#pragma once
#include <string>

namespace Mlib {

class SceneNode;

class IImposters {
public:
    virtual void create_imposter(
        SceneNode& scene_node,
        const std::string& debug_prefix) = 0;
};

}
