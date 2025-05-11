#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <compare>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct Shading {
    OrderableFixedArray<float, 3> emissive{ 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> ambient{ 1.f, 1.f, 1.f };
    OrderableFixedArray<float, 3> diffuse{ 0.8f, 0.8f, 0.8f };
    OrderableFixedArray<float, 3> specular{ 0.5f, 0.5f, 0.5f };
    float specular_exponent = 4.f;
    OrderableFixedArray<float, 3> reflectance{ 0.f, 0.f, 0.f };
    FresnelAndAmbient fresnel;
    OrderableFixedArray<float, 2> fog_distances{ default_step_distances };
    OrderableFixedArray<float, 3> fog_ambient{ 1.f, 1.f, 1.f };
    template <class Archive>
    void serialize(Archive& archive) {
        archive(emissive);
        archive(ambient);
        archive(diffuse);
        archive(specular);
        archive(specular_exponent);
        archive(reflectance);
        archive(fresnel);
        archive(fog_distances);
        archive(fog_ambient);
    }
    std::partial_ordering operator <=> (const Shading&) const = default;
};

void from_json(const nlohmann::json& j, Shading& shading);

}
