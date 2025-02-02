#pragma once
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

namespace Mlib {

template<class TData>
struct RansacOptions;

namespace Sfm {

class ProjectionToTR;

class ProjectionToTrRansac {
public:
    ProjectionToTrRansac(
        const Array<FixedArray<float, 2>>& y0,
        const Array<FixedArray<float, 2>>& y1,
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const FixedArray<float, 2>& fov_distances,
        const RansacOptions<float>& ro);

    Array<size_t> best_indices;
    std::unique_ptr<ProjectionToTR> ptr;
};

}}
