#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <vector>

namespace Mlib {

struct BillboardSequence {
    std::vector<BillboardId> billboard_ids;
    float duration;
    float final_texture_w;
};

}
