#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib::Cv {

class PointCloudResource: public ISceneNodeResource {
public:
    explicit PointCloudResource(
        const Array<TransformationMatrix<float, float, 3>>& points,
        float point_radius = 0.1f);
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
