#pragma once
#include <cmath>
#include <cstddef>

namespace Mlib {

inline float frame_index_from_animation_state(
    float elapsed,
    float duration,
    size_t number_of_frames)
{
    return std::floor(elapsed / duration * (float)number_of_frames);
}

}
