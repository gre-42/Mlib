#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Disparity/Inverse_Depth_Cost_Volume.hpp>
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


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense --intrinsic_matrix <intrinsic_matrix.m> --im0 <image0.ppm> --im1 <image1.ppm> --c0 <camera0.m> --c1 <camera1.m>",
        {},
        {"--intrinsic_matrix", "--im0", "--im1", "--c0", "--c1"});

    try {
        auto args = parser.parsed(argc, argv);

        const bool synthetic = false;

        PpmImage im0_bgr = PpmImage::load_from_file(args.named_value("--im0"));
        PpmImage im1_bgr = PpmImage::load_from_file(args.named_value("--im1"));
        if (any(im0_bgr.shape() != im1_bgr.shape())) {
            throw std::runtime_error("Image shapes differ");
        }
        if (synthetic) {
            PpmImage im_bgr{im0_bgr.copy()};
            synthetic_dense(im_bgr, im0_bgr, im1_bgr);
        }
        Array<float> im0_rgb = im0_bgr.to_float_rgb();
        Array<float> im1_rgb = im1_bgr.to_float_rgb();

        // im0_rgb = clipped(yuv2rgb(im0_rgb), 0.f, 1.f);
        // im1_rgb = clipped(yuv2rgb(im1_rgb), 0.f, 1.f);

        Array<float> im0_gray = im0_bgr.to_float_grayscale();
        Array<float> im1_gray = im1_bgr.to_float_grayscale();

        Array<float> intrinsic_matrix = Array<float>::load_txt_2d(args.named_value("--intrinsic_matrix"));

        Array<float> c0 = Array<float>::load_txt_2d(args.named_value("--c0"));
        Array<float> c1 = Array<float>::load_txt_2d(args.named_value("--c1"));

        Array<float> dc = reconstruction_in_reference(c0, c1);
        Array<float> t = t3_from_Nx4(dc, 4);
        Array<float> R = R3_from_Nx4(dc, 4);
        if (synthetic) {
            t = Array<float>{1, 0, 0};
            R = identity_array<float>(3);
        }
        Array<float> F = fundamental_from_camera(intrinsic_matrix, intrinsic_matrix, R, t);

        {
            PpmImage bmp = PpmImage::from_float_rgb(im0_rgb);
            draw_epilines_from_F(F, bmp, Rgb24::green());
            bmp.save_to_file("epilines-0.ppm");
        }
        {
            PpmImage bmp = PpmImage::from_float_rgb(im0_rgb);
            draw_inverse_epilines_from_F(F, bmp, Rgb24::green());
            bmp.save_to_file("epilines-i-0.bmp");
        }
        {
            PpmImage bmp = PpmImage::from_float_rgb(im1_rgb);
            draw_epilines_from_F(F.T(), bmp, Rgb24::green());
            bmp.save_to_file("epilines-1.bmp");
        }

        bool cache_dsi = true;
        bool use_inverse_depth = true;
        Array<float> dsi;
        float d_multiplier = 1;
        size_t search_length = 32 / d_multiplier;
        Array<float> inverse_depths = linspace<float>(1 / 10.f, 1 / 0.5f, 2 * search_length + 1);
        std::string dsi_filename =
            std::string(use_inverse_depth ? "i" : "") +
            "dsi-" +
            std::to_string(size_t(d_multiplier)) +
            ".array";
        if (!cache_dsi || !fs::exists(dsi_filename)) {
            std::cerr << dsi_filename + " does not exist, recomputing..." << std::endl;
            Array<float> disparity0;
            if (use_inverse_depth) {
                InverseDepthCostVolume vol{im0_gray.shape(), inverse_depths};
                vol.increment(
                    intrinsic_matrix,
                    inverted_homogeneous_3x4(c0),
                    inverted_homogeneous_3x4(c1),
                    im0_rgb,
                    im1_rgb);
                dsi = vol.get(1 * 3);  // 1 * 3 = min_channel_increments
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
                Array<float> x = reconstruct_disparity(disparity0 * d_multiplier, F, R, t, intrinsic_matrix);
                draw_quantiled_grayscale(x[2], 0.05, 0.95).save_to_file("xo-2.ppm");
                draw_nan_masked_grayscale(
                    disparity0,
                    -float(search_length) * d_multiplier,
                    float(search_length) * d_multiplier).save_to_file("disparity0.bmp");
            }
            std::cerr << "nanmin(dsi) " << nanmin(dsi) << std::endl;
            std::cerr << "nanmax(dsi) " << nanmax(dsi) << std::endl;
            for (size_t i = 0; i < dsi.shape(0); i += dsi.shape(0) / 5) {
                draw_nan_masked_grayscale(dsi[i], 0, 0).save_to_file("dsi-" + std::to_string(i) + ".ppm");
            }
            if (cache_dsi) {
                dsi.save_binary(dsi_filename);
            }
        } else {
            std::cerr << dsi_filename + " exists, loading..." << std::endl;
            dsi = Array<float>::load_binary(dsi_filename);
            if (!all(dsi.shape() == ArrayShape{2 * search_length + 1}.concatenated(im0_gray.shape()))) {
                throw std::runtime_error(dsi_filename + " has incorrect size");
            }
            std::cerr << "finished loading " + dsi_filename << std::endl;
        }

        // std::cerr << dsi.T()[188][303] << std::endl;
        // throw std::runtime_error("asd");

        Array<float> a;
        const std::string a_filename = dsi_filename + ".a.array";
        if (!fs::exists(a_filename)) {
            std::cerr << a_filename << " does not exist, computing..." << std::endl;
            Dm::DtamParameters params;
            a = Dm::dense_mapping(
                dsi,
                Dm::g_from_grayscale(im0_gray, params),
                params,
                false,                              // print_energy
                false);                             // print_bmps
            a.save_binary(a_filename);
        } else {
            std::cerr << a_filename << " exists, loading..." << std::endl;
            a = Array<float>::load_binary(a_filename);
        }
        draw_nan_masked_grayscale(a, 0, 0).save_to_file("a.ppm");

        Array<float> condition_number;
        Array<float> x;
        if (use_inverse_depth) {
            Array<float> depth = 1.f / a;
            draw_nan_masked_grayscale(depth, min(1.f / inverse_depths), max(1.f / inverse_depths)).save_to_file("depth.ppm");
            x = reconstruct_depth(depth, intrinsic_matrix);
        } else {
            x = reconstruct_disparity(a * d_multiplier, F, R, t, intrinsic_matrix, &condition_number);
            draw_quantiled_grayscale(condition_number, 0.05, 0.95).save_to_file("condition_number.ppm");
        }
        draw_quantiled_grayscale(x[0], 0.05, 0.95).save_to_file("x-0.ppm");
        draw_quantiled_grayscale(x[1], 0.05, 0.95).save_to_file("x-1.ppm");
        draw_quantiled_grayscale(x[2], 0.05, 0.95).save_to_file("x-2.ppm");

        {
            MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
            cams.insert(std::make_pair(std::chrono::milliseconds{0}, CameraFrame{identity_array<float>(3), zeros<float>(ArrayShape{3}), CameraFrame::undefined_kep}));
            cams.insert(std::make_pair(std::chrono::milliseconds{5}, CameraFrame{R, t, CameraFrame::undefined_kep}));
            DenseProjector::from_image(cams, 0, 1, 2, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw("dense-0-1.ppm");
            DenseProjector::from_image(cams, 0, 2, 1, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw("dense-0-2.ppm");
            DenseProjector::from_image(cams, 2, 1, 0, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw("dense-2-1.ppm");
        }

        return 0;
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
