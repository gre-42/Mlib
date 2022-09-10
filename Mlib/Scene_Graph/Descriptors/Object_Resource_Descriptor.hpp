#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

enum class AggregateMode;

struct ObjectResourceDescriptor {
    FixedArray<double, 3> position;
    float yangle;
    std::string name;
    float scale;
    AggregateMode aggregate_mode;
    bool create_impostor;
    std::map<std::string, uint32_t> supplies;
    float supplies_cooldown;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(name);
        archive(scale);
        archive(aggregate_mode);
        archive(create_impostor);
        archive(supplies);
        archive(supplies_cooldown);
    }
};

}
