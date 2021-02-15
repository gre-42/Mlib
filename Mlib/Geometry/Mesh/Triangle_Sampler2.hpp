#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <functional>

namespace Mlib {

template <class TData>
class TriangleSampler2 {
public:
    explicit TriangleSampler2(unsigned int seed)
    : rng2_{ seed }
    {}
    template <size_t tsize>
    void sample_triangle_interior(
        const FixedArray<TData, tsize>& t0,
        const FixedArray<TData, tsize>& t1,
        const FixedArray<TData, tsize>& t2,
        const TData& distance,
        const std::function<void(const TData& a, const TData& b, const TData& c)>& func)
    {
        float area = triangle_area(t0, t1, t2);
        for (size_t i = 0; i < (size_t)(area / (squared(distance / 2) * float(M_PI))); ++i) {
            // https://chrischoy.github.io/research/barycentric-coordinate-for-mesh-sampling/
            float r1q = std::sqrt(rng2_());
            float r2 = rng2_();
            float a = (1 - r1q);
            float b = r1q * (1 - r2);
            float c = r1q * r2;
            func(a, b, c);
        }
    }
    void seed(unsigned int seed) {
        rng2_.seed(seed);
    }
private:
    UniformRandomNumberGenerator<TData> rng2_;
};

}
