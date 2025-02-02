#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

struct Material;
struct Morphology;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class ColoredVertexArrayResource;

class SquareResource: public ISceneNodeResource {
public:
    SquareResource(
        const FixedArray<float, 2, 2>& square,
        const FixedArray<float, 2, 2>& uv,
        const TransformationMatrix<float, float, 3>& transformation,
        const Material& material,
        const Morphology& morphology);
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual AggregateMode get_aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
