#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

struct ResourceInstanceDescriptor {
    FixedArray<CompressedScenePos, 3> position = uninitialized;
    float yangle = 0.f;
    float scale = 1.f;  // Currently not used.
                        // Scaling can be done (per billboard_id) using
                        // the BillboardAtlasInstance::vertex_scale attribute.
    BillboardId billboard_id;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(yangle);
        archive(scale);
        archive(billboard_id);
    }
};

}
