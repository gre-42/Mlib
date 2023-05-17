#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <list>

namespace Mlib {

struct Material;
template <class TPos>
struct ColoredVertex;
class RenderingResources;

class BvhResource: public ISceneNodeResource {
public:
    BvhResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas);
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
private:
    std::list<std::shared_ptr<ColoredVertexArray<float>>> cvas_;
    Bvh<float, std::pair<const Material*, const FixedArray<ColoredVertex<float>, 3>*>, 3> bvh_;
};

}
