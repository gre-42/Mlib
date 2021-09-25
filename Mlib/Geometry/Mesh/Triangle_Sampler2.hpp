#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/N_Random_Numbers.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <functional>

namespace Mlib {

template <class TData>
class TriangleSampler2 {
public:
    explicit TriangleSampler2(unsigned int seed)
    : rng_{ seed }
    {}
    template <size_t tsize>
    void sample_triangle_interior(
        const FixedArray<TData, tsize>& t0,
        const FixedArray<TData, tsize>& t1,
        const FixedArray<TData, tsize>& t2,
        const TData& distance,
        const std::function<void(const TData& a, const TData& b, const TData& c)>& func)
    {
        TData area = triangle_area(t0, t1, t2);
        TData n = area / (squared(distance / 2) * TData(M_PI));
        n_random_numbers(n, rng_, [&func, this](){
            // https://chrischoy.github.io/research/barycentric-coordinate-for-mesh-sampling/
            TData r1q = std::sqrt(rng_());
            TData r2 = rng_();
            TData a = (1 - r1q);
            TData b = r1q * (1 - r2);
            TData c = r1q * r2;
            func(a, b, c);
        });
    }
    void seed(unsigned int seed) {
        rng_.seed(seed);
    }
private:
    UniformRandomNumberGenerator<TData> rng_;
};

}
