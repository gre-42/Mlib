#include "Projection_To_TR_Ransac.hpp"
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Stats/Ransac.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ProjectionToTrRansac::ProjectionToTrRansac(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const FixedArray<float, 2>& fov_distances,
    const RansacOptions<float>& ro)
{
    assert(y0.length() == y1.length());
    best_indices = ransac(
        y0.length(),    // nelems_large
        ro,
        [&](const Array<size_t>& indices){
            lerr() << "RANSAC ...";
            ProjectionToTR ptr{
                y0[indices],
                y1[indices],
                intrinsic_matrix,
                fov_distances};
            // static size_t i = -1;
            // ++i;
            // {
            //     StbImage3 bmp{ArrayShape{360, 640}, Rgb24::white()};
            //     ptr.draw_epilines(bmp, Rgb24::black());
            //     highlight_features(y0, bmp, 2, Rgb24::blue());
            //     highlight_features(y1, bmp, 2, Rgb24::red());
            //     highlight_features(y0[indices], bmp, 4, Rgb24::blue());
            //     highlight_features(y1[indices], bmp, 4, Rgb24::red());
            //     highlight_feature_correspondences(y0, y1, bmp);
            //     highlight_feature_correspondences(y0[indices], y1[indices], bmp, 1);
            //     bmp.save_to_file("/tmp/epilines_ransac_" + std::to_string(i) + "_" + std::to_string(ptr.ngood) + ".png");
            //     if (ptr.good()) {
            //         ptr.ke.inverted().semi_affine().to_array().save_txt_2d("/tmp/pose_" + std::to_string(i) + ".m");
            //     }
            // }
            if (ptr.good()) {
                Array<float> res{ArrayShape{y0.length()}};
                InitialReconstruction ir{y0, y1, ptr.ke, intrinsic_matrix};
                Array<FixedArray<float, 2>> p0(ir.projection_residual0());
                Array<FixedArray<float, 2>> p1(ir.projection_residual1());
                for (size_t r = 0; r < y0.length(); ++r) {
                    res(r) = (sum(squared(p0(r))) + sum(squared(p1(r)))) / 2.f;
                }
                // sqrt(res.as_column_vector()).save_txt_2d("/tmp/residual_" + std::to_string(i) + ".m");
                return res;

                // StbImage3 bmp{ArrayShape{360, 640}, Rgb24::white()};
                // ptr.draw_epilines(bmp, Rgb24::black());
                // bmp.save_to_file("/tmp/epilines_ransac_" + std::to_string(i) + "_" + std::to_string(sum(squared(ptr.fundamental_error(y0[indices], y1[indices])))) + ".png");

                // return squared(ptr.fundamental_error(y0, y1));
            } else {
                return nans<float>(ArrayShape{y0.length()});
            }
        });
    if (best_indices.initialized()) {
        ptr = std::make_unique<ProjectionToTR>(
            y0[best_indices],
            y1[best_indices],
            intrinsic_matrix,
            fov_distances);
        // static size_t i = -1;
        // ++i;
        // {
        //     StbImage3 bmp{ArrayShape{360, 640}, Rgb24::white()};
        //     ptr->draw_epilines(bmp, Rgb24::black());
        //     highlight_features(y0, bmp, 2, Rgb24::blue());
        //     highlight_features(y1, bmp, 2, Rgb24::red());
        //     highlight_features(y0[best_indices], bmp, 4, Rgb24::blue());
        //     highlight_features(y1[best_indices], bmp, 4, Rgb24::red());
        //     highlight_feature_correspondences(y0, y1, bmp);
        //     highlight_feature_correspondences(y0[best_indices], y1[best_indices], bmp, 1);
        //     bmp.save_to_file("/tmp/epilines_ransac_best_" + std::to_string(i) + "_" + std::to_string(ptr->ngood) + ".png");
        //     if (ptr->good()) {
        //         ptr->ke.inverted().semi_affine().to_array().save_txt_2d("/tmp/pose_best_" + std::to_string(i) + ".m");
        //     }
        // }
    } else {
        ptr = nullptr;
    }
}
