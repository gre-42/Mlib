#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>
#include <nlohmann/json_fwd.hpp>

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

struct FresnelAndAmbient {
    FresnelReflectance reflectance;
    OrderableFixedArray<float, 3> ambient{ 0.f, 0.f, 0.f };
    template <class Archive>
    void serialize(Archive& archive) {
        archive(reflectance);
        archive(ambient);
    }
    std::partial_ordering operator <=> (const FresnelAndAmbient&) const = default;
};

void from_json(const nlohmann::json& j, FresnelAndAmbient& f);

}

template <>
struct std::hash<Mlib::FresnelReflectance>
{
    std::size_t operator() (const Mlib::FresnelReflectance& a) const {
        return Mlib::hash_combine(
            a.min,
            a.max,
            a.exponent);
    }
};

template <>
struct std::hash<Mlib::FresnelAndAmbient>
{
    std::size_t operator() (const Mlib::FresnelAndAmbient& a) const {
        return Mlib::hash_combine(
            a.reflectance,
            a.ambient);
    }
};
