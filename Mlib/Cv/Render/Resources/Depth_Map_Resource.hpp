#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib::Cv {

class DepthMapResource: public ISceneNodeResource {
public:
    DepthMapResource(
        const Array<float>& rgb_picture,
        const Array<float>& depth_picture,
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        float cos_threshold = 0.f);
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
