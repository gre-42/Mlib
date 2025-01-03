#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

struct Morphology {
    PhysicsMaterial physics_material;
    OrderableFixedArray<float, 2> center_distances{ default_step_distances };
    float max_triangle_distance = INFINITY;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(physics_material);
        archive(center_distances);
        archive(max_triangle_distance);
    }
};

Morphology& operator += (Morphology& a, PhysicsMaterial b);
Morphology& operator -= (Morphology& a, PhysicsMaterial b);
Morphology operator + (const Morphology& a, PhysicsMaterial b);
Morphology operator - (const Morphology& a, PhysicsMaterial b);

}
