#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib { namespace Sfm {

class NormalizedProjection {
public:
    NormalizedProjection(const Array<FixedArray<float, 2>>& p);
    Array<FixedArray<float, 2>> normalized_y(const Array<FixedArray<float, 2>>& y);
    TransformationMatrix<float, 2> denormalized_intrinsic_matrix(const TransformationMatrix<float, 2>& m);
    TransformationMatrix<float, 2> normalized_intrinsic_matrix(const TransformationMatrix<float, 2>& m);
    Array<FixedArray<float, 2>> yn;
    TransformationMatrix<float, 2> N;
};

}}
