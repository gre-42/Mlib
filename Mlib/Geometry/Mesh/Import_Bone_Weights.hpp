#pragma once
#include <list>
#include <memory>

namespace Mlib {

struct AnimatedColoredVertexArrays;
struct ColoredVertexArray;

void import_bone_weights(
    AnimatedColoredVertexArrays& dest,
    const AnimatedColoredVertexArrays& source,
    float max_distance);

}
