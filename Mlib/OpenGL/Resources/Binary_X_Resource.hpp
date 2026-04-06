#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

struct Material;
struct Morphology;
class ColoredVertexArrayResource;

class BinaryXResource: public ISceneNodeResource {
public:
    BinaryXResource(
        const FixedArray<float, 2, 2>& square,
        const Material& material_0,
        const Material& material_90,
        const Morphology& morphology_0,
        const Morphology& morphology_90);
    ~BinaryXResource();
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual AggregateMode get_aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_0_;
    std::shared_ptr<ColoredVertexArrayResource> rva_90_;

};

}
