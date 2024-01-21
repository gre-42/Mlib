#pragma once
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <compare>

namespace Mlib {

struct Shading {
    OrderableFixedArray<float, 3> emissive{0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> ambient{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> diffuse{0.8f, 0.8f, 0.8f};
    OrderableFixedArray<float, 3> specular{0.5f, 0.5f, 0.5f};
    float specular_exponent = 4.f;
    FresnelAndAmbient fresnel;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(emissive);
        archive(ambient);
        archive(diffuse);
        archive(specular);
        archive(specular_exponent);
        archive(fresnel);
    }
    std::partial_ordering operator <=> (const Shading&) const = default;
};

}
