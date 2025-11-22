#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> get_triangles_around(
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius);

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> get_triangles_around(
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius);

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> get_triangles_around(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius);

}
