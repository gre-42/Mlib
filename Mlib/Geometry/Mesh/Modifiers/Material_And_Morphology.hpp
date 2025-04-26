#pragma once
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <compare>

namespace Mlib {

struct MaterialAndMorphology {
    Material material;
    Morphology morphology;
    std::partial_ordering operator <=> (const MaterialAndMorphology&) const = default;
};

}
