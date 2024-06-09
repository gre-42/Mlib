#pragma once
#include <cstdint>
#include <vector>

namespace Mlib {

struct BillboardSequence {
    std::vector<uint32_t> billboard_ids;
    float duration;
    float final_texture_layer;
};

}
