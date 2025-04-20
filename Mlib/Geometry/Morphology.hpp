#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <compare>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

struct Morphology {
    PhysicsMaterial physics_material;
    SquaredStepDistances center_distances2{ default_step_distances2 };
    float max_triangle_distance = INFINITY;
    std::partial_ordering operator <=> (const Morphology&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(physics_material);
        archive(center_distances2);
        archive(max_triangle_distance);
    }
};

Morphology& operator += (Morphology& a, PhysicsMaterial b);
Morphology& operator -= (Morphology& a, PhysicsMaterial b);
Morphology operator + (const Morphology& a, PhysicsMaterial b);
Morphology operator - (const Morphology& a, PhysicsMaterial b);

}
