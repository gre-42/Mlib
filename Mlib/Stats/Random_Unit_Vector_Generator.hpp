#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <cstdint>

namespace Mlib {

template <class TData, size_t tndim>
class RandomUnitVectorGenerator {
public:
    explicit RandomUnitVectorGenerator(unsigned int seed)
        : rng_{ seed }
    {}
    FixedArray<TData, tndim> operator () () {
        while (true) {
            FixedArray<TData, tndim> result = uninitialized;
            for (auto& v : result.flat_iterable()) {
                v = rng_();
            }
            auto len = std::sqrt(sum(squared(result)));
            if (len > 1e-5) {
                return result / len;
            }
        }
    }
    FixedArray<TData, tndim> surface(const FixedArray<TData, tndim>& surface_normal) {
        auto res = (*this)();
        auto h = dot0d(res, surface_normal);
        if (h < 0) {
            res -= 2 * h * surface_normal;
        }
        return res;
    }
    FixedArray<TData, tndim> optional_surface(const std::optional<FixedArray<TData, tndim>>& surface_normal) {
        return surface_normal.has_value()
            ? surface(*surface_normal)
            : (*this)();
    }
    void seed(unsigned int seed) {
        rng_.seed(seed);
    }
private:
    FastNormalRandomNumberGenerator<TData> rng_;
};

}
