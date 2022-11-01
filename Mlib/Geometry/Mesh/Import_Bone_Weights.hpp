#pragma once
#include <list>
#include <memory>

namespace Mlib {

struct AnimatedColoredVertexArrays;
template <class TPos>
class ColoredVertexArray;

void import_bone_weights(
    AnimatedColoredVertexArrays& dest,
    const AnimatedColoredVertexArrays& source,
    float max_distance);

}
