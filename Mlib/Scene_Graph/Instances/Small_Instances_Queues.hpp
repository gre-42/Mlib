#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Instances/Deferred_Vertex_Arrays_And_Instances.hpp>
#include <Mlib/Scene_Graph/Instances/Instance_Location.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

enum class ExternalRenderPassType;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct SceneGraphConfig;
class IGpuVertexData;

class SmallInstancesQueues {
public:
    SmallInstancesQueues(
        ExternalRenderPassType main_render_pass,
        const std::set<ExternalRenderPassType>& black_render_passes);
    ~SmallInstancesQueues();
    void insert(
        const std::list<std::shared_ptr<IGpuVertexData>>& scvas,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config);
    std::map<ExternalRenderPassType, VertexDatasAndSortedInstances> sorted_instances() const;
private:
    ExternalRenderPassType main_render_pass_;
    VertexDatasAndInstances invisible_queue_;
    SortableDeferredVertexDatasAndSortableInstances standard_queue_;
    std::map<ExternalRenderPassType, VertexDatasAndInstances> black_queues_;
};

}
