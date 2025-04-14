#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

enum class NormalType;
class ColoredVertexArrayResource;

class HeightMapResource: public ISceneNodeResource {
public:
    HeightMapResource(
        const Array<float>& rgb_picture,
        const Array<float>& height_picture,
        const TransformationMatrix<float, float, 2>& normalization_matrix,
        NormalType normal_type);
    virtual ~HeightMapResource() override;

    // Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;

    // Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
