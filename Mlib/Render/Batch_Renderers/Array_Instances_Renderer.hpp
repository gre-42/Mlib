#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class RenderingResources;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;

class ArrayInstancesRenderer : public IInstancesRenderer {
    ArrayInstancesRenderer(const ArrayInstancesRenderer& other) = delete;
    ArrayInstancesRenderer& operator=(const ArrayInstancesRenderer& other) = delete;
public:
    explicit ArrayInstancesRenderer(RenderingResources& rendering_resources);
    virtual ~ArrayInstancesRenderer() override;
    virtual bool is_initialized() const override;
    virtual void invalidate() override;
    virtual void update_instances(
        const FixedArray<double, 3>& offset,
        const std::list<TransformedColoredVertexArray>& instances_queue,
        TaskLocation task_location) override;
    virtual void render_instances(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;

private:
    RenderingResources& rendering_resources_;
    mutable std::shared_ptr<ColoredVertexArrayResource> next_rcva_;
    mutable std::unique_ptr<RenderableColoredVertexArray> rcvai_;
    mutable std::unique_ptr<RenderableColoredVertexArray> next_rcvai_;
    mutable FixedArray<double, 3> offset_;
    FixedArray<double, 3> next_offset_;
    mutable std::mutex mutex_;
    bool is_initialized_;
};

}
