#pragma once

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

bool get_has_per_instance_continuous_texture_layer(const ColoredVertexArray<float>& cva);

bool get_has_discrete_atlas_texture_layer(const ColoredVertexArray<float>& cva);

}
