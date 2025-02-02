#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib::Cv {

template <class TData>
class RigidMotionSampler {
public:
    RigidMotionSampler(
        const TransformationMatrix<TData, TData, 2>& ki_0,
        const TransformationMatrix<TData, TData, 2>& ki_1,
        const TransformationMatrix<TData, TData, 3>& ke_1_0,
        const Array<TData>& depth0,
        const ArrayShape& shape1)
    : iki_0_{inv(ki_0.affine()).value()},
      T_{ki_1.project(ke_1_0.semi_affine())},
      depth0_(depth0),
      shape1_(shape1)
    {}
    inline FixedArray<TData, 3> point_in_reference(size_t r, size_t c) const {
        FixedArray<size_t, 2> id_s{r, c};
        return dot1d(iki_0_, homogenized_3(i2a(id_s))) * depth0_(r, c);
    }
    inline bool sample_destination(const FixedArray<TData, 3>& pr, BilinearInterpolator<TData>& bi) const {
        FixedArray<TData, 3> dp3 = dot1d(T_, homogenized_4(pr));
        FixedArray<TData, 2> id_d{a2fi(FixedArray<TData, 2>{dp3(0) / dp3(2), dp3(1) / dp3(2)})};
        return bilinear_interpolation(id_d(0), id_d(1), shape1_(0), shape1_(1), bi);
    }
    inline bool sample_destination(size_t r, size_t c, BilinearInterpolator<TData>& bi) const {
        FixedArray<TData, 3> pr = point_in_reference(r, c);
        return sample_destination(pr, bi);
    }
private:
    FixedArray<TData, 3, 3> iki_0_;
    FixedArray<TData, 3, 4> T_;
    Array<TData> depth0_;
    ArrayShape shape1_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
