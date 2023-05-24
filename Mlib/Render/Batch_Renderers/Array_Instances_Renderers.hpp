#pragma once
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <map>

namespace Mlib {

class ArrayInstancesRenderers: public IInstancesRenderers {
public:
    virtual void invalidate() override;
    virtual std::shared_ptr<IInstancesRenderer> get_instances_renderer(ExternalRenderPassType render_pass) const override;
private:
    mutable std::map<ExternalRenderPassType, std::shared_ptr<IInstancesRenderer>> renderers_;
};

}
