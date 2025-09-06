#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

class IntersectableResource: public ISceneNodeResource {
public:
    explicit IntersectableResource(
        std::list<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables);

    // Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const override;
private:
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables_;
};

}
