#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

class ColoredVertexArrayResource;
struct Material;
struct Morphology;

class BlendingXResource: public ISceneNodeResource {
public:
    BlendingXResource(
        const FixedArray<float, 2, 2>& square,
        const FixedArray<Material, 2>& materials,
        const FixedArray<Morphology, 2>& morphology);
    ~BlendingXResource();
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
private:
    FixedArray<std::shared_ptr<ColoredVertexArrayResource>, 2> rva_;
    // Square is stored to facilitate creating depth-sorted nodes during instantiation.
    FixedArray<float, 2, 2> square_;
    FixedArray<AggregateMode, 2> aggregate_modes_;
};

}
