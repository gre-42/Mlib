#pragma once

namespace Mlib {

class SceneNode;

class IImposters {
public:
    virtual void create_imposter(SceneNode& scene_node) = 0;
};

}
