#pragma once
#include <Mlib/Math/Interp.hpp>

namespace Mlib {

template <class TData>
TData n_random_numbers_correction_factor(const TData& n) {
    // Interpolation table, computed with the program at the end of this file.
    static const Interp<TData> interp{
        {0.f, 0.05, 0.075, 0.1125, 0.16875, 0.253125, 0.379688, 0.569531,
            0.854297, 1.28145, 1.92217, 2.88325, 4.32488, 6.48732, 9.73098,
            14.5965, 21.8947, 32.842, 49.2631, 73.8946},
        {1.f, 0.980696, 0.968693, 0.946344, 0.921824, 0.891393, 0.848769, 0.792251,
            0.723697, 0.644143, 0.580853, 0.559309, 0.538969, 0.525868, 0.51717,
            0.51143, 0.507617, 0.505021, 0.503331, 0.50224},
        OutOfRangeBehavior::EXPLICIT,
        1.f,
        0.5f};
    return interp(n);
}

/** Generate on average n random numbers, where n can be a floating-point number.
 */
template <class TData, class TUniformRandomNumberGenerator, class TOperation>
void n_random_numbers(const TData& n, TUniformRandomNumberGenerator& rng, const TOperation& op) {
    TData thr = n_random_numbers_correction_factor(n) * n;
    TData pp = 0.f;
    while (true) {
        TData rn = rng();
        pp += rn;
        if (pp >= thr) {
            break;
        }
        op();
    }
}

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
