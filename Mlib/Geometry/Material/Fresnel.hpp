#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <compare>

namespace Mlib {

struct FresnelReflectance {
    float min = 0.f;
    float max = 0.f;
    float exponent = 0.f;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(min);
        archive(max);
        archive(exponent);
    }
    std::partial_ordering operator <=> (const FresnelReflectance&) const = default;
};

struct FresnelAndAmbience {
    FresnelReflectance reflectance;
    OrderableFixedArray<float, 3> ambience{ 0.f, 0.f, 0.f };
    template <class Archive>
    void serialize(Archive& archive) {
        archive(reflectance);
        archive(ambience);
    }
    std::partial_ordering operator <=> (const FresnelAndAmbience&) const = default;
};

}
