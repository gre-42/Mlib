#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib::Sfm {

template <class TData>
class RigidMotionSampler {
public:
    RigidMotionSampler(const TransformationMatrix<TData, 2>& ki, const TransformationMatrix<TData, 3>& ke, const Array<TData>& depth)
    : iki_{inv(ki.affine())},
      T_{ki.project(ke.semi_affine())},
      depth_(depth)
    {}
    inline FixedArray<TData, 3> point_in_reference(size_t r, size_t c) const {
        FixedArray<size_t, 2> id_s{r, c};
        return dot1d(iki_, homogenized_3(i2a(id_s))) * depth_(r, c);
    }
    inline bool sample_destination(const FixedArray<TData, 3>& pr, BilinearInterpolator<TData>& bi) const {
        FixedArray<TData, 3> dp3 = dot1d(T_, homogenized_4(pr));
        FixedArray<TData, 2> id_d{a2fi(FixedArray<TData, 2>{dp3(0) / dp3(2), dp3(1) / dp3(2)})};
        return bilinear_interpolation(id_d(0), id_d(1), depth_.shape(), bi);
    }
    inline bool sample_destination(size_t r, size_t c, BilinearInterpolator<TData>& bi) const {
        FixedArray<TData, 3> pr = point_in_reference(r, c);
        return sample_destination(pr, bi);
    }
private:
    FixedArray<TData, 3, 3> iki_;
    FixedArray<TData, 3, 4> T_;
    Array<TData> depth_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
