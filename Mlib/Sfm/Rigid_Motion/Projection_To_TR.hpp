#pragma once
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

namespace Mlib {

class StbImage3;
struct Rgb24;

namespace Sfm {

class ProjectionToTR {
public:
    ProjectionToTR(
        const Array<FixedArray<float, 2>>& y0,
        const Array<FixedArray<float, 2>>& y1,
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const FixedArray<float, 2>& fov_distances);

    bool good() const;
    InitialReconstruction initial_reconstruction() const;
    Array<float> fundamental_error(
        const Array<FixedArray<float, 2>>& y0,
        const Array<FixedArray<float, 2>>& y1) const;
    void draw_epilines(StbImage3& image, const Rgb24& color) const;

    TransformationMatrix<float, float, 3> ke;
    size_t ngood;
private:
    NormalizedProjection np;
    TransformationMatrix<float, float, 2> kin;
    FixedArray<float, 3, 3> Fn;
    FixedArray<float, 3, 3> En;
    EssentialMatrixToTR e2tr;
};

}}
