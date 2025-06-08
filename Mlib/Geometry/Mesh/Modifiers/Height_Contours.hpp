#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
class ColoredVertexArray;

std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_edge(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height);

std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_vertex(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height);

}
