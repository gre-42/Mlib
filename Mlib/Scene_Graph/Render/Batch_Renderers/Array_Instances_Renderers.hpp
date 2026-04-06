#pragma once
#include <Mlib/Scene_Graph/Render/Batch_Renderers/IInstances_Renderer.hpp>
#include <map>

namespace Mlib {

class IGpuObjectFactory;
class IGpuVertexArrayRenderer;

class ArrayInstancesRenderers: public IInstancesRenderers {
public:
    explicit ArrayInstancesRenderers(
        const IGpuObjectFactory& gpu_object_factory,
        IGpuVertexArrayRenderer& gpu_vertex_array_renderer);
    virtual void invalidate() override;
    virtual std::shared_ptr<IInstancesRenderer> get_instances_renderer(ExternalRenderPassType render_pass) const override;
private:
    const IGpuObjectFactory& gpu_object_factory_;
    IGpuVertexArrayRenderer& gpu_vertex_array_renderer_;
    mutable std::map<ExternalRenderPassType, std::shared_ptr<IInstancesRenderer>> renderers_;
};

}
