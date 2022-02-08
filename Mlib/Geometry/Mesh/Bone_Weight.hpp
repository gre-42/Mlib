#pragma once
#include <cstddef>

namespace Mlib {

struct BoneWeight {
    size_t bone_index;
    float weight;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(bone_index, weight);
    }
};

}
