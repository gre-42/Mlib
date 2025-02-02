#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;

UUVector<FixedArray<CompressedScenePos, 2>> get_map_outer_contour(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
