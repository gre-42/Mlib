#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Deferred_Vertex_Arrays_And_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Vertex_Data_And_Sorted_Instances.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <atomic>
#include <optional>

namespace Mlib {

class IGpuObjectFactory;
class VertexArraysWithDynamicInstances;
class IGpuVertexArray;
class IGpuVertexArrayRenderer;

class ArrayInstancesRenderer : public IInstancesRenderer {
    ArrayInstancesRenderer(const ArrayInstancesRenderer& other) = delete;
    ArrayInstancesRenderer& operator=(const ArrayInstancesRenderer& other) = delete;
public:
    ArrayInstancesRenderer(
        const IGpuObjectFactory& gpu_object_factory,
        IGpuVertexArrayRenderer& gpu_renderer);
    virtual ~ArrayInstancesRenderer() override;
    virtual bool is_initialized() const override;
    virtual void invalidate() override;
    virtual void update_instances(
        const FixedArray<ScenePos, 3>& offset,
        VertexDatasAndSortedInstances&& instances_queue,
        TaskLocation task_location) override;
    virtual void render_instances(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual FixedArray<ScenePos, 3> offset() const override;

private:
    mutable std::unique_ptr<VertexArraysWithDynamicInstances> rcva_;
    mutable std::unique_ptr<VertexArraysWithDynamicInstances> next_rcva_;
    mutable std::unique_ptr<std::list<std::shared_ptr<IGpuVertexArray>>> rcvai_;
    mutable std::unique_ptr<std::list<std::shared_ptr<IGpuVertexArray>>> next_rcvai_;
    TaskLocation next_task_location_;
    mutable std::optional<VertexDatasAndSortedInstances> next_instances_queue_;
    mutable FixedArray<ScenePos, 3> offset_;
    FixedArray<ScenePos, 3> next_offset_;
    mutable FastMutex mutex_;
    bool is_initialized_;
    IGpuVertexArrayRenderer& gpu_renderer_;
};

}
