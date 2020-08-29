#include "Normalized_Projection.hpp"
#include <Mlib/Geometry/Normalized_Points.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

NormalizedProjection::NormalizedProjection(const Array<float>& y)
{
    assert(y.ndim() == 3);
    assert(y.shape(2) == 3);
    NormalizedPoints npo(
        true,   // preserve_aspect_ratio
        true);  // centered
    y.shape().erased_last().foreach([&](const ArrayShape& s){
        npo.add_point(y[s]);
    });
    N = npo.normalization_matrix();
    yn = normalized_y(y);
}

Array<float> NormalizedProjection::normalized_y(const Array<float>& y) {
    assert(y.ndim() == 3);
    assert(y.shape(2) == 3);
    Array<float> yn(y.shape());
    y.shape().erased_last().foreach([&](const ArrayShape& s){
        yn[s] = dot1d(N, y[s]);
    });
    return yn;
}

Array<float> NormalizedProjection::denormalized_intrinsic_matrix(const Array<float>& m) {
    assert(all(m.shape() == ArrayShape{3, 3}));
    Array<float> result;
    result = lstsq_chol(N.casted<double>(), m.casted<double>()).casted<float>();
    return result;
}

Array<float> NormalizedProjection::normalized_intrinsic_matrix(const Array<float>& m) {
    assert(all(m.shape() == ArrayShape{3, 3}));
    Array<float> result;
    result = dot(N, m);
    return result;
}

void NormalizedProjection::print_min_max() const {
    std::cerr << min_x << " - " << max_x << std::endl;
    std::cerr << min_y << " - " << max_y << std::endl;
}
