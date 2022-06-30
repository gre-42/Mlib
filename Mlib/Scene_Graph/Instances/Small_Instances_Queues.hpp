#pragma once
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
struct ColoredVertexArray;
struct SceneGraphConfig;

class SmallInstancesQueues {
public:
    explicit SmallInstancesQueues(const std::set<ExternalRenderPassType>& black_render_passes);
    ~SmallInstancesQueues();
    void insert(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config);
    std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray>> sorted_instances();
private:
    std::list<TransformedColoredVertexArray> invisible_queue_;
    std::list<std::pair<float, TransformedColoredVertexArray>> standard_queue_;
    std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray*>> black_queues_;
};

}
