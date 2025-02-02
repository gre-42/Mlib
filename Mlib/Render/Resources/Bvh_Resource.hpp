#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <list>

namespace Mlib {

struct Material;
struct Morphology;
template <class TPos>
struct ColoredVertex;
class RenderingResources;

struct BvhResourcePayload {
    std::shared_ptr<ColoredVertexArray<float>> cva;
    const FixedArray<ColoredVertex<float>, 3>& triangle;
};

class BvhResource: public ISceneNodeResource {
public:
    BvhResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas);
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
private:
    std::list<std::shared_ptr<ColoredVertexArray<float>>> cvas_;
    Bvh<float, 3, BvhResourcePayload> bvh_;
};

}
