#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>


namespace Mlib {

class BaseProjector {

public:
    static inline Array<float> identity_scale_matrix() {
        return identity_array<float>(3);
    }

    BaseProjector(
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& scale_matrix = identity_scale_matrix())
    :i0_(i0),
     i1_(i1),
     iz_(iz),
     scale_matrix_(scale_matrix) {
        assert(all(scale_matrix_.shape() == ArrayShape{3, 3}));
    }

protected:
    ArrayShape x2i(const Array<float>& x) {
        return a2i(project(x));
    }

    Array<float> x2fi(const Array<float>& x) {
        return a2fi(project(x));
    }

    Array<float> project(const Array<float>& x) {
        assert(all(x.shape() == ArrayShape{3}));
        Array<float> sliced{x(i0_), x(i1_)};
        return dot1d(scale_matrix_, homogenized_3(sliced)).row_range(0, 2);
    }

    float zcoord(const Array<float>& x) {
        assert(all(x.shape() == ArrayShape{3}));
        return x(iz_);
    }

    size_t i0_;
    size_t i1_;
    size_t iz_;

protected:
    Array<float> scale_matrix_;
};

}
