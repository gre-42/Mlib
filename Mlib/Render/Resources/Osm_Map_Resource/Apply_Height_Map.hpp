#pragma once
#include <list>
#include <map>
#include <set>
#include <string>

namespace Mlib {

class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t n>
class TransformationMatrix;
template <class TData>
class Array;
struct Node;
struct Way;
template <class TData>
class Interp;
enum class EntranceType;
class HeightBinding;

void apply_height_map(
    const TriangleList& tl_terrain,
    const std::map<EntranceType, std::set<OrderableFixedArray<float, 2>>>& entrances,
    float tunnel_height,
    float extrude_air_support_amount,
    std::list<FixedArray<float, 3>*>& in_vertices,
    std::set<const FixedArray<float, 3>*>& vertices_to_delete,
    const Array<float>& heightmap,
    const TransformationMatrix<float, 2>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, HeightBinding>& height_bindings,
    float street_node_smoothness,
    const Interp<float>& layer_height);

}
