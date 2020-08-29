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
        const Array<float>& y0,
        const Array<float>& y1,
        const Array<float>& intrinsic_matrix,
        float threshold,
        const RansacOptions<float>& ro);

    Array<size_t> best_indices;
    std::unique_ptr<ProjectionToTR> ptr;
};

}}
