#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Halton_Sequence.hpp>
#include <Mlib/Stats/N_Random_Numbers.hpp>
#include <functional>

namespace Mlib {

template <class TPosition>
class TriangleSampler2 {
public:
    using I = funpack_t<TPosition>;
    explicit TriangleSampler2(unsigned int seed)
        : rng_{ seed }
    {}
    template <size_t tsize>
    void sample_triangle_interior(
        const FixedArray<TPosition, 3, tsize>& t,
        const TPosition& distance,
        const std::function<void(const FixedArray<I, 3>& c)>& func)
    {
        I area = triangle_area(funpack(t));
        auto n = (I)area / (squared(distance / 2) * I(M_PI));
        n_random_numbers(n, rng_, [&func, this](){
            // https://chrischoy.github.io/research/barycentric-coordinate-for-mesh-sampling/
            I r1q = std::sqrt(rng_());
            I r2 = rng_();
            func(FixedArray<I, 3>{
                (1 - r1q),
                r1q * (1 - r2),
                r1q * r2});
        });
    }
    void seed(unsigned int seed) {
        rng_.seed(seed);
    }
private:
    HybridHaltonSequence<I> rng_;
};

}
