#pragma once

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

template <class TPos>
void remove_degenerate_triangles(ColoredVertexArray<TPos>& cva);

}
