#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib { namespace Sfm {

class EpilineDirection {
public:
    EpilineDirection(
        size_t r,
        size_t c,
        const Array<float>& F,
        bool l1_normalization = true,
        float good_threshold = 1e-5)
    : center0(i2a(ArrayShape{r, c}))
    {
        assert(all(F.shape() == ArrayShape{3, 3}));
        find_epiline(F, Mlib::homogenized_3(center0), center1, v1);
        good = sum(squared(v1 / F(2, 2))) >= good_threshold;
        // v = center - epipole;
        v1 /= (l1_normalization ? max(abs(v1)) : std::sqrt(sum(squared(v1))));
    }
    Array<float> center0;
    Array<float> center1;
    Array<float> v1;
    bool good;
};

}}
