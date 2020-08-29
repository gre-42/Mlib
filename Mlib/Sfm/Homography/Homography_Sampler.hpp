#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>

namespace Mlib { namespace Sfm {

template <class TData>
class HomographySampler {
public:
    HomographySampler(const Array<TData>& homography)
    : H(homography) {}
    bool sample_destination(size_t r, size_t c, const ArrayShape& shape, BilinearInterpolator<TData>& bi) const {
        FixedArray<size_t, 2> id_s{r, c};
        FixedArray<TData, 2> id_d{a2fi(dehomogenized_2(apply_homography(H, homogenized_3(i2a(id_s)))))};
        return bilinear_interpolation(id_d(0), id_d(1), shape, bi);
    }
private:
    FixedArray<TData, 3, 3> H;
};

}}
