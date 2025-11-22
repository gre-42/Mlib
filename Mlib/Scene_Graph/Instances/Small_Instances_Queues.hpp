#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

enum class ExternalRenderPassType;
struct TransformedColoredVertexArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
class ColoredVertexArray;
struct SceneGraphConfig;

class SmallInstancesQueues {
public:
    explicit SmallInstancesQueues(
        ExternalRenderPassType main_render_pass,
        const std::set<ExternalRenderPassType>& black_render_passes);
    ~SmallInstancesQueues();
    void insert(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config);
    std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray>> sorted_instances();
private:
    ExternalRenderPassType main_render_pass_;
    std::list<TransformedColoredVertexArray> invisible_queue_;
    std::list<std::pair<float, TransformedColoredVertexArray>> standard_queue_;
    std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray*>> black_queues_;
};

}
