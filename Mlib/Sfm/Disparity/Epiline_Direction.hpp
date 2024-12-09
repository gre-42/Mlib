#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib::Sfm {

class EpilineDirection {
public:
    EpilineDirection(
        size_t r,
        size_t c,
        const FixedArray<float, 3, 3>& F,
        bool l1_normalization = true,
        float good_threshold = 1e-5)
        : center0{ i2a(ArrayShape{r, c}) }
        , center1{ uninitialized}
        , v1{ uninitialized }
    {
        find_epiline(F, center0, center1, v1);
        good = sum(squared(v1 / F(2, 2))) >= good_threshold;
        // v = center - epipole;
        v1 /= (l1_normalization ? max(abs(v1)) : std::sqrt(sum(squared(v1))));
    }
    FixedArray<float, 2> center0;
    FixedArray<float, 2> center1;
    FixedArray<float, 2> v1;
    bool good;
};

}
