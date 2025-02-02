#pragma once
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <map>

namespace Mlib {

class RenderingResources;

class ArrayInstancesRenderers: public IInstancesRenderers {
public:
    explicit ArrayInstancesRenderers(RenderingResources& rendering_resources);
    virtual void invalidate() override;
    virtual std::shared_ptr<IInstancesRenderer> get_instances_renderer(ExternalRenderPassType render_pass) const override;
private:
    RenderingResources& rendering_resources_;
    mutable std::map<ExternalRenderPassType, std::shared_ptr<IInstancesRenderer>> renderers_;
};

}
