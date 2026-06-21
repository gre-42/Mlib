#pragma once
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <cstdint>

namespace Mlib {

struct BoneWeight {
    uint32_t bone_index;
    float weight;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(bone_index);
        archive(weight);
    }
};

}
