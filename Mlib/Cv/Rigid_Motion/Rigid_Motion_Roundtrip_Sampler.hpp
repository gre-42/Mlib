#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib::Cv {

template <class TData>
class RigidMotionRoundtripSampler {
public:
    RigidMotionRoundtripSampler(
        const TransformationMatrix<TData, TData, 2>& ki_r,
        const TransformationMatrix<TData, TData, 2>& ki_l,
        const TransformationMatrix<TData, TData, 3>& ke,
        const Array<TData>& depth_r,
        const Array<TData>& depth_l)
    : iki_r_(inv(ki_r.affine()).value()),
      iki_l_(inv(ki_l.affine()).value()),
      T_r_(ki_l.project(ke.semi_affine())),
      T_l_(ki_r.project(ke.inverted().semi_affine())),
      depth_r_(depth_r),
      depth_l_(depth_l)
    {}
    bool roundtrip_position(size_t r, size_t c, FixedArray<TData, 2>& pos_r_) const {
        FixedArray<size_t, 2> id_s{r, c};
        FixedArray<TData, 3> l_res3{dot(T_r_, homogenized_4(dot(iki_r_, homogenized_3(i2a(id_s))) * depth_r_(r, c)))};
        FixedArray<TData, 2> id_d{a2fi(FixedArray<TData, 2>{l_res3(0) / l_res3(2), l_res3(1) / l_res3(2)})};
        BilinearInterpolator<TData> bi;
        if (bilinear_interpolation(id_d(0), id_d(1), depth_l_.shape(0), depth_l_.shape(1), bi)) {
            TData dl = bi(depth_l_);
            FixedArray<TData, 3> r_res3{dot(T_l_, homogenized_4(dot(iki_l_, homogenized_3(fi2a(id_d))) * dl))};
            pos_r_ = a2fi(FixedArray<TData, 2>{r_res3(0) / r_res3(2), r_res3(1) / r_res3(2)});
            return true;
        } else {
            return false;
        }
    }
private:
    FixedArray<TData, 3, 3> iki_r_;
    FixedArray<TData, 3, 3> iki_l_;
    FixedArray<TData, 3, 4> T_r_;
    FixedArray<TData, 3, 4> T_l_;
    Array<TData> depth_r_;
    Array<TData> depth_l_;
};

}
