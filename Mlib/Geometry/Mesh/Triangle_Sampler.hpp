#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <functional>

namespace Mlib {

template <class TData>
class TriangleSampler {
public:
    explicit TriangleSampler(unsigned int seed)
    : rng2_{ seed, 0.f, 1.2f }
    {}
    template <size_t tshape>
    void sample_triangle_interior(
        const FixedArray<TData, tshape>& t0,
        const FixedArray<TData, tshape>& t1,
        const FixedArray<TData, tshape>& t2,
        const TData& distance,
        const std::function<void(const TData& a, const TData& b, const TData& c)>& func)
    {
        float dist_a = std::sqrt(std::min(sum(squared(t1 - t0)), sum(squared(t2 - t0))));
        float dist_b = std::sqrt(std::min(sum(squared(t1 - t0)), sum(squared(t2 - t1))));
        for (float a = 0.01f; a < 0.99f; a += distance / dist_a) {
            for (float b = 0.01f; b < 1.f - a; b += distance / dist_b) {
                float aa = a + rng2_() * distance / dist_a;
                float bb = b + rng2_() * distance / dist_b;
                float c = 1 - aa - bb;
                if (c < 0 || aa < 0 || bb < 0) {
                    continue;
                }
                func(aa, bb, c);
            }
        }
    }
    void seed(unsigned int seed) {
        rng2_.seed(seed);
    }
private:
    NormalRandomNumberGenerator<TData> rng2_;
};

}
