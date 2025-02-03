#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <atomic>

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
        const FixedArray<ScenePos, 3>& offset,
        const std::list<TransformedColoredVertexArray>& instances_queue,
        TaskLocation task_location) override;
    virtual void render_instances(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;
    virtual FixedArray<ScenePos, 3> offset() const override;

private:
    RenderingResources& rendering_resources_;
    mutable std::shared_ptr<ColoredVertexArrayResource> rcva_;
    mutable std::shared_ptr<ColoredVertexArrayResource> next_rcva_;
    mutable std::unique_ptr<RenderableColoredVertexArray> rcvai_;
    mutable std::unique_ptr<RenderableColoredVertexArray> next_rcvai_;
    mutable FixedArray<ScenePos, 3> offset_;
    FixedArray<ScenePos, 3> next_offset_;
    mutable FastMutex mutex_;
    bool is_initialized_;
};

}
