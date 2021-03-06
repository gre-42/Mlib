#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class ColoredVertexArrayResource;
struct Material;

class BlendingXResource: public SceneNodeResource {
public:
    BlendingXResource(
        const FixedArray<float, 2, 2>& square,
        const Material& material_0,
        const Material& material_90);
    ~BlendingXResource();
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
private:
    FixedArray<std::shared_ptr<ColoredVertexArrayResource>, 2> rva_;
    // Square is stored to facilitate creating depth-sorted nodes during instantiation.
    FixedArray<float, 2, 2> square_;

};

}
