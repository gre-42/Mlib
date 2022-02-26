#pragma once
#include <compare>

namespace Mlib {

enum class PhysicsMaterial;

template <class T>
struct TypedMesh {
    PhysicsMaterial physics_material;
    T mesh;
    std::strong_ordering operator <=> (const TypedMesh&) const = default;
};

}
