#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class ResolveCollisionType {
    PENALTY,
    SEQUENTIAL_PULSES
};

inline ResolveCollisionType resolve_collission_type_from_string(const std::string& resolve_collission_type) {
    if (resolve_collission_type == "penalty") {
        return ResolveCollisionType::PENALTY;
    } else if (resolve_collission_type == "sequential_pulses") {
        return ResolveCollisionType::SEQUENTIAL_PULSES;
    } else {
        throw std::runtime_error("Unknown resolve collision type");
    }
}

}
