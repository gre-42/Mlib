#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

struct ResourceInstanceDescriptor {
    FixedArray<double, 3> position;
    float yangle = 0.f;
    float scale = 1.f;  // Currently not used.
                        // Scaling can be done (per billboard_id) using
                        // the BillboardAtlasInstance::vertex_scale attribute.
    uint32_t billboard_id;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(yangle);
        archive(scale);
        archive(billboard_id);
    }
};

}
