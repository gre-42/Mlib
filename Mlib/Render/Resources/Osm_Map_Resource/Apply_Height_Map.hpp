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

void apply_height_map(
    const TriangleList& tl_terrain,
    const std::set<OrderableFixedArray<float, 2>>& tunnel_entrances,
    float tunnel_height,
    std::list<FixedArray<float, 3>*>& in_vertices,
    std::set<const FixedArray<float, 3>*>& vertices_to_delete,
    const Array<float>& heightmap,
    const TransformationMatrix<float, 2>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    float street_node_smoothness,
    const Interp<float>& layer_height);

}
