#pragma once
#include <compare>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class T>
struct TypedMesh {
    PhysicsMaterial physics_material;
    T mesh;
    std::strong_ordering operator <=> (const TypedMesh&) const = default;
};

}
