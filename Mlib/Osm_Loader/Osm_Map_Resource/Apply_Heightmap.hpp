#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData>
class Array;
struct Node;
struct Way;
enum class EntranceType;
class NodeHeightBinding;
template <class TPos>
class VertexHeightBinding;
enum class TerrainType;
template <class EntityType>
class EntityTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;

void apply_heightmap(
    const TerrainTypeTriangleList& tl_terrain,
    const std::map<EntranceType, std::set<OrderableFixedArray<double, 2>>>& entrances,
    float tunnel_height,
    float extrude_air_support_amount,
    std::list<FixedArray<double, 3>*>& in_vertices,
    std::set<const FixedArray<double, 3>*>& vertices_to_delete,
    const Array<double>& heightmap,
    const Array<bool>& heightmap_mask,
    size_t heightmap_extension,
    const TransformationMatrix<double, double, 2>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<double, 2>, NodeHeightBinding>& node_height_bindings,
    const std::map<const FixedArray<double, 3>*, VertexHeightBinding<double>>& vertex_height_bindings,
    float street_node_smoothness,
    size_t street_node_smoothing_iterations,
    const Interp<double>& layer_heights);

}
