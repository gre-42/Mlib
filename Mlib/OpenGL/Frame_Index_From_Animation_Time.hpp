#pragma once
#include <cstdint>

namespace Mlib {

inline float frame_index_from_animation_state(
    float elapsed,
    float duration,
    uint32_t number_of_frames)
{
    return elapsed / duration * (float)number_of_frames;
}

}
