#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resource.hpp>

namespace Mlib {

enum class NormalType;
class ColoredVertexArrayResource;

class HeightMapResource: public SceneNodeResource {
public:
    HeightMapResource(
        const Array<float>& rgb_picture,
        const Array<float>& height_picture,
        const TransformationMatrix<float, float, 2>& normalization_matrix,
        NormalType normal_type);
    ~HeightMapResource();
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
