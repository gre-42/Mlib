#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Instances_Renderer.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class RenderingResources;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;

class ArrayInstancesRenderer: public InstancesRenderer {
public:
    ArrayInstancesRenderer(const ArrayInstancesRenderer& other) = delete;
    ArrayInstancesRenderer& operator = (const ArrayInstancesRenderer& other) = delete;
    explicit ArrayInstancesRenderer();
    virtual ~ArrayInstancesRenderer() override;
    virtual bool is_initialized() const override;
    virtual void invalidate() override;
    virtual void update_instances(
        const FixedArray<double, 3>& offset,
        const std::list<TransformedColoredVertexArray>& instances_queue) override;
    virtual void render_instances(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rcva_;
    std::unique_ptr<RenderableColoredVertexArray> rcvai_;
    FixedArray<double, 3> offset_;
    mutable std::mutex mutex_;
    bool is_initialized_;
};

}
