#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

enum class AggregateMode;

struct ObjectResourceDescriptor {
    FixedArray<CompressedScenePos, 3> position = uninitialized;
    float yangle;
    VariableAndHash<std::string> name;
    float scale;
    AggregateMode aggregate_mode;
    bool create_imposter;
    uint32_t max_imposter_texture_size;
    std::map<std::string, uint32_t> supplies;
    float supplies_cooldown;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position);
        archive(yangle);
        archive(name);
        archive(scale);
        archive(aggregate_mode);
        archive(create_imposter);
        archive(max_imposter_texture_size);
        archive(supplies);
        archive(supplies_cooldown);
    }
};

}
