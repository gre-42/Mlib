#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

class AnimatedColoredVertexArrayResource: public ISceneNodeResource {
public:
    explicit AnimatedColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& acvas);
    ~AnimatedColoredVertexArrayResource();

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const;

private:
    std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
};

}
