#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

struct ObjectResourceDescriptor {
    FixedArray<double, 3> position;
    std::string name;
    float scale = 1.f;
    std::optional<std::map<std::string, uint32_t>> supplies;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(name);
        archive(scale);
        archive(supplies);
    }
};

}
