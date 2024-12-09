#include "Normalized_Projection.hpp"
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

NormalizedProjection::NormalizedProjection(const Array<FixedArray<float, 2>>& y)
    : N{ uninitialized }
{
    NormalizedPointsFixed<float> npo(
        ScaleMode::PRESERVE_ASPECT_RATIO,
        OffsetMode::CENTERED);
    for (const auto& p : y.flat_iterable()) {
        npo.add_point(p);
    }
    N = npo.normalization_matrix();
    yn = normalized_y(y);
}

Array<FixedArray<float, 2>> NormalizedProjection::normalized_y(const Array<FixedArray<float, 2>>& y) const {
    return y.applied([this](const FixedArray<float, 2>& p){return N.transform(p);});
}

TransformationMatrix<float, float, 2> NormalizedProjection::denormalized_intrinsic_matrix(const TransformationMatrix<float, float, 2>& m) const {
    return N.inverted_scaled() * m;
}

TransformationMatrix<float, float, 2> NormalizedProjection::normalized_intrinsic_matrix(const TransformationMatrix<float, float, 2>& m) const {
    return N * m;
}
