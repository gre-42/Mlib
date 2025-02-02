#pragma once
#include <cstddef>

namespace Mlib {

inline float frame_index_from_animation_state(
    float elapsed,
    float duration,
    size_t number_of_frames)
{
    return elapsed / duration * (float)number_of_frames;
}

}
