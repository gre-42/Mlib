#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resource.hpp>

namespace Mlib {

class ColoredVertexArrayResource;
struct Material;

class BlendingXResource: public SceneNodeResource {
public:
    BlendingXResource(
        const FixedArray<float, 2, 2>& square,
        const FixedArray<Material, 2>& materials);
    ~BlendingXResource();
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
private:
    FixedArray<std::shared_ptr<ColoredVertexArrayResource>, 2> rva_;
    // Square is stored to facilitate creating depth-sorted nodes during instantiation.
    FixedArray<float, 2, 2> square_;
    FixedArray<AggregateMode, 2> aggregate_modes_;
};

}
