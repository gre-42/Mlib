#pragma once
#include <compare>

namespace Mlib {

struct Fresnel {
    float min;
    float max;
    float exponent;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(min);
        archive(max);
        archive(exponent);
    }
    std::partial_ordering operator <=> (const Fresnel&) const = default;
};

}
