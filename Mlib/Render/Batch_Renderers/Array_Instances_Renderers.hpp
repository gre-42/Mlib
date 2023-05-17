#pragma once
#include <Mlib/Scene_Graph/Batch_Renderers/Instances_Renderer.hpp>
#include <map>

namespace Mlib {

class ArrayInstancesRenderers: public InstancesRenderers {
public:
    virtual void invalidate() override;
    virtual std::shared_ptr<InstancesRenderer> get_instances_renderer(ExternalRenderPassType render_pass) const override;
private:
    mutable std::map<ExternalRenderPassType, std::shared_ptr<InstancesRenderer>> renderers_;
};

}
