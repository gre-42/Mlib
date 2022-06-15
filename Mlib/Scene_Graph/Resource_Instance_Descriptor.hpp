#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

struct ObjectResourceDescriptor {
    FixedArray<double, 3> position;
    std::string name;
    float scale = 1.f;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(name);
        archive(scale);
    }
};

struct ResourceInstanceDescriptor {
    FixedArray<double, 3> position;
    float yangle = 0.f;
    float scale = 1.f;  // Currently not used
    uint32_t billboard_id;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(yangle);
        archive(billboard_id);
    }
};

}
