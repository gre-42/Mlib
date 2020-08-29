#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>

namespace Mlib { namespace Sfm {

template <class TData>
class RigidMotionSampler {
public:
    RigidMotionSampler(const Array<TData>& ki, const Array<TData>& ke, const Array<TData>& depth)
    : iki_{inv(ki)},
      T_{dot(ki, ke)},
      depth_{depth}
    {}
    bool sample_destination(size_t r, size_t c, BilinearInterpolator<TData>& bi) const {
        FixedArray<size_t, 2> id_s{r, c};
        FixedArray<TData, 3> res3{dot(T_, homogenized_4(dot(iki_, homogenized_3(i2a(id_s))) * depth_(r, c)))};
        FixedArray<TData, 2> id_d{a2fi(FixedArray<TData, 2>{res3(0) / res3(2), res3(1) / res3(2)})};
        return bilinear_interpolation(id_d(0), id_d(1), depth_.shape(), bi);
    }
private:
    FixedArray<TData, 3, 3> iki_;
    FixedArray<TData, 3, 4> T_;
    Array<TData> depth_;
};

}}
