#include "Projection_To_TR_Ransac.hpp"
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Stats/Ransac.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ProjectionToTrRansac::ProjectionToTrRansac(
    const Array<float>& y0,
    const Array<float>& y1,
    const Array<float>& intrinsic_matrix,
    float threshold,
    const RansacOptions<float>& ro)
{
    assert(all(y0.shape() == y1.shape()));
    best_indices = ransac(
        y0.shape(0),    // nelems_large
        ro,
        [&](const Array<size_t>& indices){
            std::cerr << "RANSAC ..." << std::endl;
            ProjectionToTR ptr{
                y0[indices],
                y1[indices],
                intrinsic_matrix,
                threshold};
            if (ptr.good()) {
                Array<float> res{ArrayShape{y0.shape(0)}};
                InitialReconstruction ir{y0, y1, ptr.R, ptr.t, intrinsic_matrix};
                Array<float> p0{ir.projection_residual0()};
                Array<float> p1{ir.projection_residual1()};
                for (size_t r = 0; r < indices.length(); ++r) {
                    res(r) = sum(squared(p0[r])) + sum(squared(p1[r]));
                }
                return res;
            } else {
                return nans<float>(ArrayShape{y0.shape(0)});
            }
        });
    if (best_indices.initialized()) {
        ptr = std::make_unique<ProjectionToTR>(
            y0[best_indices],
            y1[best_indices],
            intrinsic_matrix,
            threshold);
    } else {
        ptr = nullptr;
    }
}
