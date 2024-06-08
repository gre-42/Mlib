#pragma once

namespace Mlib {

struct AnimatedColoredVertexArrays;

void import_bone_weights(
    AnimatedColoredVertexArrays& dest,
    const AnimatedColoredVertexArrays& source,
    float max_distance);

}
