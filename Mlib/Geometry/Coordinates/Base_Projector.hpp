#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>


namespace Mlib {

class BaseProjector {

public:
    static inline FixedArray<float, 3, 3> nan_scale_matrix() {
        return fixed_nans<float, 3, 3>();
    }

    BaseProjector(
        size_t i0,
        size_t i1,
        size_t iz,
        const FixedArray<float, 3, 3>& scale_matrix = nan_scale_matrix())
    :i0_(i0),
     i1_(i1),
     iz_(iz),
     scale_matrix_(scale_matrix)
    {}

protected:
    FixedArray<size_t, 2> x2i(const FixedArray<float, 3>& x) {
        if (all(Mlib::isnan(scale_matrix_))) {
            return a2i(project(x));
        } else {
            return na2i(project(x));
        }
    }

    FixedArray<float, 2> x2fi(const FixedArray<float, 3>& x) {
        if (all(Mlib::isnan(scale_matrix_))) {
            return a2fi(project(x));
        } else {
            return na2fi(project(x));
        }
    }

    FixedArray<float, 2> project(const FixedArray<float, 3>& x) {
        FixedArray<float, 2> sliced{x(i0_), x(i1_)};
        if (all(Mlib::isnan(scale_matrix_))) {
            return sliced;
        } else {
            return dot1d(scale_matrix_, homogenized_3(sliced)).template row_range<0, 2>();
        }
    }

    float zcoord(const FixedArray<float, 3>& x) {
        return x(iz_);
    }

    size_t i0_;
    size_t i1_;
    size_t iz_;

protected:
    FixedArray<float, 3, 3> scale_matrix_;
};

}
