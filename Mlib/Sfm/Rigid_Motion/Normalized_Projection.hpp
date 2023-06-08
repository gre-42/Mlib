#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib::Sfm {

class NormalizedProjection {
public:
    NormalizedProjection(const Array<FixedArray<float, 2>>& p);
    Array<FixedArray<float, 2>> normalized_y(const Array<FixedArray<float, 2>>& y) const;
    TransformationMatrix<float, float, 2> denormalized_intrinsic_matrix(const TransformationMatrix<float, float, 2>& m) const;
    TransformationMatrix<float, float, 2> normalized_intrinsic_matrix(const TransformationMatrix<float, float, 2>& m) const;
    Array<FixedArray<float, 2>> yn;
    TransformationMatrix<float, float, 2> N;
};

}
