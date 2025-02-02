#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Synthetic_Dense.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense_mapping --intrinsic_matrix <intrinsic_matrix.m> --im0 <image0.png> --im1 <image1.png> --c0 <camera0.m> --c1 <camera1.m>",
        {},
        {"--intrinsic_matrix", "--im0", "--im1", "--c0", "--c1"});

    try {
        auto args = parser.parsed(argc, argv);

        const bool synthetic = false;

        StbImage3 im0_bgr = StbImage3::load_from_file(args.named_value("--im0"));
        StbImage3 im1_bgr = StbImage3::load_from_file(args.named_value("--im1"));
        if (any(im0_bgr.shape() != im1_bgr.shape())) {
            throw std::runtime_error("Image shapes differ");
        }
        if (synthetic) {
            StbImage3 im_bgr{im0_bgr.copy()};
            synthetic_dense(im_bgr, im0_bgr, im1_bgr);
        }
        Array<float> im0_rgb = im0_bgr.to_float_rgb();
        Array<float> im1_rgb = im1_bgr.to_float_rgb();

        // im0_rgb = clipped(yuv2rgb(im0_rgb), 0.f, 1.f);
        // im1_rgb = clipped(yuv2rgb(im1_rgb), 0.f, 1.f);

        Array<float> im0_gray = im0_bgr.to_float_grayscale();
        Array<float> im1_gray = im1_bgr.to_float_grayscale();

        TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d(args.named_value("--intrinsic_matrix")) } };

        TransformationMatrix<float, float, 3> c0{ FixedArray<float, 4, 4>{ Array<float>::load_txt_2d(args.named_value("--c0")) } };
        TransformationMatrix<float, float, 3> c1{ FixedArray<float, 4, 4>{ Array<float>::load_txt_2d(args.named_value("--c1")) } };

        Array<float> dc = reconstruction_in_reference(c0.affine().to_array(), c1.affine().to_array());
        TransformationMatrix<float, float, 3> ke{ FixedArray<float, 4, 4>{dc} };
        if (synthetic) {
            ke.t = FixedArray<float, 3>{1.f, 0.f, 0.f};
            ke.R = fixed_identity_array<float, 3>();
        }
        FixedArray<float, 3, 3> F = fundamental_from_camera(intrinsic_matrix, intrinsic_matrix, ke);

        {
            StbImage3 bmp = StbImage3::from_float_rgb(im0_rgb);
            draw_epilines_from_F(F, bmp, Rgb24::green());
            bmp.save_to_file("epilines-0.png");
        }
        {
            StbImage3 bmp = StbImage3::from_float_rgb(im0_rgb);
            draw_inverse_epilines_from_F(F, bmp, Rgb24::green());
            bmp.save_to_file("epilines-i-0.png");
        }
        {
            StbImage3 bmp = StbImage3::from_float_rgb(im1_rgb);
            draw_epilines_from_F(F.T(), bmp, Rgb24::green());
            bmp.save_to_file("epilines-1.png");
        }

        bool cache_dsi = true;
        bool use_inverse_depth = true;
        Array<float> dsi;
        float d_multiplier = 1;
        size_t search_length = (size_t)(32.f / d_multiplier);
        Array<float> inverse_depths = linspace<float>(1 / 4.f, 1 / 0.5f, 2 * search_length + 1);
        std::string dsi_filename =
            std::string(use_inverse_depth ? "i" : "") +
            "dsi-" +
            std::to_string(size_t(d_multiplier)) +
            ".array";
        if (!cache_dsi || !fs::exists(dsi_filename)) {
            lerr() << dsi_filename + " does not exist, recomputing...";
            Array<float> disparity0;
            if (use_inverse_depth) {
                InverseDepthCostVolumeAccumulator vol{im0_gray.shape(), inverse_depths};
                vol.increment(
                    intrinsic_matrix,
                    c0,
                    c1,
                    im0_rgb,
                    im1_rgb);
                dsi.move() = vol.get(1 * 3)->dsi();  // 1 * 3 = min_channel_increments
                disparity0 = argmin(dsi, 0).take(inverse_depths);
            } else {
                if (false) {
                    disparity0 = compute_disparity_gray_single_pixel(
                        im0_gray,
                        im1_gray,
                        F,
                        search_length,
                        false,  // true = l1_normalization
                        &dsi,
                        d_multiplier);
                } else {
                    disparity0 = compute_disparity_rgb_single_pixel(
                        im0_rgb,
                        im1_rgb,
                        F,
                        search_length,
                        false,  // true = l1_normalization
                        &dsi,
                        d_multiplier);
                }
            }
            if (!use_inverse_depth) {
                Array<float> x = reconstruct_disparity(disparity0 * d_multiplier, F, intrinsic_matrix, ke);
                draw_quantiled_grayscale(x[2], 0.05f, 0.95f).save_to_file("xo-2.png");
                draw_nan_masked_grayscale(
                    disparity0,
                    -float(search_length) * d_multiplier,
                    float(search_length) * d_multiplier).save_to_file("disparity0.bmp");
            }
            lerr() << "nanmin(dsi) " << nanmin(dsi);
            lerr() << "nanmax(dsi) " << nanmax(dsi);
            for (size_t i = 0; i < dsi.shape(0); i += dsi.shape(0) / 5) {
                draw_nan_masked_grayscale(dsi[i], 0, 0).save_to_file("dsi-" + std::to_string(i) + ".png");
            }
            if (cache_dsi) {
                dsi.save_binary(dsi_filename);
            }
        } else {
            lerr() << dsi_filename + " exists, loading...";
            dsi = Array<float>::load_binary(dsi_filename);
            if (!all(dsi.shape() == ArrayShape{2 * search_length + 1}.concatenated(im0_gray.shape()))) {
                throw std::runtime_error(dsi_filename + " has incorrect size");
            }
            lerr() << "finished loading " + dsi_filename;
        }

        // lerr() << dsi.T()[188][303];
        // throw std::runtime_error("asd");

        Array<float> ai;
        const std::string ai_filename = dsi_filename + ".ai.array";
        if (!fs::exists(ai_filename)) {
            lerr() << ai_filename << " does not exist, computing...";
            Dm::DtamParameters params;
            Dm::DenseMapping dm{
                g_from_grayscale(im0_gray, params.edge_image_config_),
                CostVolumeParameters(),
                params,
                false,                              // print_energy
                false};                             // print_bmps
            dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
            dm.iterate_atmost(SIZE_MAX);
            ai = dm.interpolated_inverse_depth_image();
            ai.save_binary(ai_filename);
        } else {
            lerr() << ai_filename << " exists, loading...";
            ai = Array<float>::load_binary(ai_filename);
        }
        draw_nan_masked_grayscale(ai, 0, 0).save_to_file("ai.png");

        Array<float> condition_number;
        Array<float> x;
        if (use_inverse_depth) {
            Array<float> depth = 1.f / ai;
            draw_nan_masked_grayscale(depth, min(1.f / inverse_depths), max(1.f / inverse_depths)).save_to_file("depth.png");
            x = reconstruct_depth(depth, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity());
        } else {
            x = reconstruct_disparity(ai * d_multiplier, F, intrinsic_matrix, ke, &condition_number);
            draw_quantiled_grayscale(condition_number, 0.05f, 0.95f).save_to_file("condition_number.png");
        }
        draw_quantiled_grayscale(x[0], 0.05f, 0.95f).save_to_file("x-0.png");
        draw_quantiled_grayscale(x[1], 0.05f, 0.95f).save_to_file("x-1.png");
        draw_quantiled_grayscale(x[2], 0.05f, 0.95f).save_to_file("x-2.png");

        {
            MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
            cams.insert(std::make_pair(std::chrono::milliseconds{0}, CameraFrame{ TransformationMatrix<float, float, 3>::identity() }));
            cams.insert(std::make_pair(std::chrono::milliseconds{5}, CameraFrame{ ke }));
            DenseProjector::from_image(cams, 0, 1, 2, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-0-1.png");
            DenseProjector::from_image(cams, 0, 2, 1, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-0-2.png");
            DenseProjector::from_image(cams, 2, 1, 0, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-2-1.png");
        }

        return 0;
    } catch (const CommandLineArgumentError& e) {
        lerr() << e.what();
        return 1;
    }
}
