#include "Normalized_Projection.hpp"
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

NormalizedProjection::NormalizedProjection(const Array<FixedArray<float, 2>>& y)
{
    assert(y.ndim() == 2);
    NormalizedPointsFixed npo(
        ScaleMode::PRESERVE_ASPECT_RATIO,
        OffsetMode::CENTERED);
    y.shape().foreach([&](const ArrayShape& s){
        npo.add_point(FixedArray<float, 2>{y(s)});
    });
    N = npo.normalization_matrix();
    yn = normalized_y(y);
}

Array<FixedArray<float, 2>> NormalizedProjection::normalized_y(const Array<FixedArray<float, 2>>& y) const {
    assert(y.ndim() == 2);
    Array<FixedArray<float, 2>> yn(y.shape());
    y.shape().foreach([&](const ArrayShape& s){
        yn(s) = N.transform(FixedArray<float, 2>{y(s)});
    });
    return yn;
}

TransformationMatrix<float, 2> NormalizedProjection::denormalized_intrinsic_matrix(const TransformationMatrix<float, 2>& m) const {
    return N.inverted_scaled() * m;
}

TransformationMatrix<float, 2> NormalizedProjection::normalized_intrinsic_matrix(const TransformationMatrix<float, 2>& m) const {
    return N * m;
}
