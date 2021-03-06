#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class AnimatedColoredVertexArrayResource: public SceneNodeResource {
public:
    explicit AnimatedColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& acvas);
    ~AnimatedColoredVertexArrayResource();

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const;

private:
    std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
};

}
