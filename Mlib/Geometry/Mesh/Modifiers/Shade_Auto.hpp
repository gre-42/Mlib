#pragma once

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

template <class TPos>
void shade_auto(ColoredVertexArray<TPos>& cvas, float seam_angle);

}
