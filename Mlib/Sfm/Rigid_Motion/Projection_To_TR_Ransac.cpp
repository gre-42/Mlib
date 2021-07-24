#include "Projection_To_TR_Ransac.hpp"
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Stats/Ransac.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ProjectionToTrRansac::ProjectionToTrRansac(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
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
            // static size_t i = -1;
            // ++i;
            // {
            //     StbImage bmp{ArrayShape{360, 640}, Rgb24::white()};
            //     ptr.draw_epilines(bmp, Rgb24::black());
            //     highlight_features(y0, bmp, 2, Rgb24::blue());
            //     highlight_features(y1, bmp, 2, Rgb24::red());
            //     highlight_feature_correspondences(y0, y1, bmp);
            //     bmp.save_to_file("/tmp/epilines_ransac_" + std::to_string(i) + "_" + std::to_string(ptr.ngood) + ".png");
            //     ptr.fundamental_error(y0, y1).as_column_vector().save_txt_2d("/tmp/fundamental_error_" + std::to_string(i) + ".m");
            //     if (ptr.good()) {
            //         ptr.ke.inverted().affine().to_array().save_txt_2d("/tmp/pose_" + std::to_string(i) + ".m");
            //     }
            // }
            if (ptr.good()) {
                // Array<float> res{ArrayShape{y0.shape(0)}};
                // InitialReconstruction ir{y0, y1, ptr.ke, intrinsic_matrix};
                // Array<FixedArray<float, 2>> p0{ir.projection_residual0()};
                // Array<FixedArray<float, 2>> p1{ir.projection_residual1()};
                // for (size_t r = 0; r < y0.shape(0); ++r) {
                //     res(r) = sum(squared(p0(r))) + sum(squared(p1(r)));
                // }
                // return res;

                // StbImage bmp{ArrayShape{360, 640}, Rgb24::white()};
                // ptr.draw_epilines(bmp, Rgb24::black());
                // bmp.save_to_file("/tmp/epilines_ransac_" + std::to_string(i) + "_" + std::to_string(sum(squared(ptr.fundamental_error(y0[indices], y1[indices])))) + ".png");

                return squared(ptr.fundamental_error(y0, y1));
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
