#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

struct TransformedColoredVertexArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
class ColoredVertexArray;
struct SceneGraphConfig;
enum class ExternalRenderPassType;

enum class InvisibilityHandling {
    RAISE,
    SKIP
};

class LargeInstancesQueue {
public:
    explicit LargeInstancesQueue(ExternalRenderPassType render_pass);
    ~LargeInstancesQueue();
    void insert(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        InvisibilityHandling invisibility_handling);
    const std::list<TransformedColoredVertexArray>& queue() const;
    ExternalRenderPassType render_pass() const;
private:
    ExternalRenderPassType render_pass_;
    std::list<TransformedColoredVertexArray> queue_;
};

}
