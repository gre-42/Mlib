#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>

namespace Mlib::Sfm {

template <class TData>
class HomographySampler {
public:
    HomographySampler(const FixedArray<TData, 3, 3>& homography)
    : H(homography) {}
    bool sample_destination(size_t r, size_t c, size_t nrows, size_t ncols, BilinearInterpolator<TData>& bi) const {
        FixedArray<size_t, 2> id_s{r, c};
        FixedArray<TData, 2> id_d{a2fi(apply_homography(H, i2a(id_s)))};
        return bilinear_interpolation(id_d(0), id_d(1), nrows, ncols, bi);
    }
private:
    FixedArray<TData, 3, 3> H;
};

}
