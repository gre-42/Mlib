#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
class GroupAndName;
struct MeshAndPosition {
    std::shared_ptr<ColoredVertexArray<float>> cva;
    FixedArray<CompressedScenePos, 3> position;
};
template <class TPos>
std::list<MeshAndPosition> cluster_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<FixedArray<TPos, 3>(const ColoredVertexArray<TPos>&)>& get_cluster_center,
    const GroupAndName& prefix);

}
