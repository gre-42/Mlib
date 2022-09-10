#pragma once

namespace Mlib {

class SceneNode;

class IImpostors {
public:
    virtual void create_impostor(SceneNode& scene_node) = 0;
};

}
