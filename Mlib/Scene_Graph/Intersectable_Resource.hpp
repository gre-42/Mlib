#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

class IntersectableResource: public ISceneNodeResource {
public:
    explicit IntersectableResource(
        std::list<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables);

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const override;
private:
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables_;
};

}
