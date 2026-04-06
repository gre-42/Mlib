#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ColoredVertexArrayResource;

class ProctreeFileResource: public ISceneNodeResource {
public:
    ProctreeFileResource(
        const std::string& filename,
        const LoadMeshConfig<float>& cfg);
    ~ProctreeFileResource();

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual AggregateMode get_aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;
};

}
