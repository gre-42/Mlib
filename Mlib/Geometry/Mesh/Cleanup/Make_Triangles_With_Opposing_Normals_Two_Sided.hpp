#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

template <class TPos>
void make_triangles_with_opposing_normals_two_sided(
    ColoredVertexArray<TPos>& cva,
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& result);

}
