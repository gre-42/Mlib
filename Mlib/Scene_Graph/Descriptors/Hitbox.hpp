#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

struct Hitbox {
    FixedArray<double, 3> position;
    float yangle = 0.f;
    float scale = 1.f;  // Currently not used
    BillboardId billboard_id;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(position);
        archive(yangle);
        archive(scale);
        archive(billboard_id);
    }
};

}
