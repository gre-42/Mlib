#pragma once
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

namespace Mlib { namespace Sfm {

class ProjectionToTR {
public:
    ProjectionToTR(
        const Array<float>& y0,
        const Array<float>& y1,
        const Array<float>& intrinsic_matrix,
        float threshold);

    bool good() const;
    InitialReconstruction initial_reconstruction() const;

    Array<float> R;
    Array<float> t;
    size_t ngood;
private:
    Array<float> y;
    NormalizedProjection np;
    Array<float> kin;
    Array<float> Fn;
    Array<float> En;
    EssentialMatrixToTR e2tr;
};

}}
