#pragma once
#include <Mlib/Math/Interp.hpp>

namespace Mlib {

template <class TData>
TData n_random_numbers_correction_factor(const TData& n) {
    // Interpolation table, computed with the program at the end of this file.
    static const Interp<TData> interp{
        {0.f, 0.05f, 0.075f, 0.1125f, 0.16875f, 0.253125f, 0.379688f, 0.569531f,
            0.854297f, 1.28145f, 1.92217f, 2.88325f, 4.32488f, 6.48732f, 9.73098f,
            14.5965f, 21.8947f, 32.842f, 49.2631f, 73.8946f},
        {1.f, 0.980696f, 0.968693f, 0.946344f, 0.921824f, 0.891393f, 0.848769f, 0.792251f,
            0.723697f, 0.644143f, 0.580853f, 0.559309f, 0.538969f, 0.525868f, 0.51717f,
            0.51143f, 0.507617f, 0.505021f, 0.503331f, 0.50224f},
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
