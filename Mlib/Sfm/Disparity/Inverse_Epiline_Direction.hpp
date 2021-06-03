#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>

namespace Mlib { namespace Sfm {

class InverseEpilineDirection {
public:
    InverseEpilineDirection(
        size_t r,
        size_t c,
        const FixedArray<float, 3, 3>& F,
        float good_threshold = 1e-5)
    : center0{i2a(ArrayShape{r, c})}
    {
        // y'^T F y = 0
        // y'^T (F y) = 0
        // y' k = 0
        // [y0', y1'] * [k0; k1] + c2 = 0
        // l * [k0, k1] * [k0; k1] + c2 = 0
        // l = -k2 / (k0^2 + k1^2)
        FixedArray<float, 3> k = dot1d(F, homogenized_3(center0));
        float l = -k(2) / (squared(k(0)) + squared(k(1)));
        FixedArray<float, 3> y{k(0) * l, k(1) * l, 1};
        FixedArray<float, 3> n = dot1d(F.vH(), y);

        good = (sum(squared(n)) > 1e-5);
        if (good) {
            v0 = FixedArray<float, 2>{-n(1), n(0)};
            v0 /= max(abs(v0));
        }
    }
    FixedArray<float, 2> center0;
    FixedArray<float, 2> v0;
    bool good;
};

}}
