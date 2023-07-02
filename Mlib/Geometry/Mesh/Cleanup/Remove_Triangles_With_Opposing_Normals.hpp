#pragma once

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

template <class TPos>
void remove_triangles_with_opposing_normals(ColoredVertexArray<TPos>& cva);

}
