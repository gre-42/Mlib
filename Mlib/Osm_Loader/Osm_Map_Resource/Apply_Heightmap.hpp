#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace Mlib {

template <class TPos>
class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;
enum class EntranceType;
class NodeHeightBinding;
template <class TPos>
class VertexHeightBinding;
enum class TerrainType;
template <class EntityType>
class EntityTypeTriangleList;
using TerrainTypeTriangleList = EntityTypeTriangleList<TerrainType>;
class HeightSampler;

void apply_heightmap(
    const TerrainTypeTriangleList& tl_terrain,
    const std::map<EntranceType, std::set<OrderableFixedArray<CompressedScenePos, 2>>>& entrances,
    CompressedScenePos tunnel_height,
    CompressedScenePos extrude_air_support_amount,
    std::list<FixedArray<CompressedScenePos, 3>*>& in_vertices,
    std::set<const FixedArray<CompressedScenePos, 3>*>& vertices_to_delete,
    const HeightSampler& height_sampler,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    const std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    float street_node_smoothness,
    size_t street_node_smoothing_iterations,
    const Interp<double>& layer_heights);

}
