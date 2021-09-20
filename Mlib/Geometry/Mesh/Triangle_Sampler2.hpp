#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Interp.hpp>
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
        // Interpolation table, computed with the program at the end of this file.
        static const Interp<float> interp{
            {0.f, 0.05, 0.075, 0.1125, 0.16875, 0.253125, 0.379688, 0.569531,
             0.854297, 1.28145, 1.92217, 2.88325, 4.32488, 6.48732, 9.73098,
             14.5965, 21.8947, 32.842, 49.2631, 73.8946},
            {1.f, 0.980696, 0.968693, 0.946344, 0.921824, 0.891393, 0.848769, 0.792251,
             0.723697, 0.644143, 0.580853, 0.559309, 0.538969, 0.525868, 0.51717,
             0.51143, 0.507617, 0.505021, 0.503331, 0.50224},
            OutOfRangeBehavior::EXPLICIT,
            1.f,
            0.5f};
        float area = triangle_area(t0, t1, t2);
        float frac = area / (squared(distance / 2) * float(M_PI));
        float thr = interp(frac) * frac;
        float pp = 0.f;
        while (true) {
            pp += rng2_();
            if (pp >= thr) {
                break;
            }
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

// #include <Mlib/Math/Math.hpp>
// #include <Mlib/Math/Optimize/Newton_1D.hpp>
// #include <Mlib/Stats/Random_Number_Generators.hpp>
// #include <iostream>
// 
// using namespace Mlib;
// 
// static const size_t N = 1000000;
// 
// int main() {
//     for (float thr = 0.05f; thr < 100.f; thr *= 1.5f) {
//         auto f = [thr](float a){
//             UniformRandomNumberGenerator<float> rng{ 0 };
//             float result = 0.f;
//             size_t nn = 0;
//             for (size_t i = 0; i < N; ++i) {
//                 size_t n = 0;
//                 float pp = 0.f;
//                 while (true) {
//                     pp += rng();
//                     if (pp >= a * thr) {
//                         break;
//                     }
//                     ++n;
//                 }
//                 nn += n;
//             }
//             return nn / thr / N - 1.f;
//         };
//         float h = 0.01f;
//         auto df = [&thr, &h, &f](const double& x){return (f(x + h) - f(x - h)) / (2 * h);};
//         std::cout << thr << ", " << newton_1d(f, df, 0.5f, float{ 1e-4 }, 100) << std::endl;
//     }
//     return 0;
// }
// 
// // g++ -I. test_rng_sum.cpp -g -O3 -std=c++20 -o test_rng_sum && gdb -ex=r ./test_rng_sum
