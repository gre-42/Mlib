#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

struct Material;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class ColoredVertexArrayResource;

class SquareResource: public SceneNodeResource {
public:
    SquareResource(
        const FixedArray<float, 2, 2>& square,
        const TransformationMatrix<float, float, 3>& transformation,
        const Material& material);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual AggregateMode aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
