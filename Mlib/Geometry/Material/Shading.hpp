#pragma once
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <compare>

namespace Mlib {

struct Shading {
    OrderableFixedArray<float, 3> emissivity{0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> ambience{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> diffusivity{0.8f, 0.8f, 0.8f};
    OrderableFixedArray<float, 3> specularity{0.5f, 0.5f, 0.5f};
    float specular_exponent = 4.f;
    FresnelAndAmbience fresnel;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(emissivity);
        archive(ambience);
        archive(diffusivity);
        archive(specularity);
        archive(specular_exponent);
        archive(fresnel);
    }
    std::partial_ordering operator <=> (const Shading&) const = default;
};

}
