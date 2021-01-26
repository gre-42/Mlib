#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Base_Projector.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_In_Box.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_On_Line.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR_Ransac.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <Mlib/Stats/RansacOptions.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;

static const bool only_features2k = false;

CorrespondingFeaturesOnLine get_cfol(
    const Array<float>& im1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    const std::string& bmp_filename)
{
    Array<float> hr1d = harris_response_1d(im1, F);
    Array<float> features2k = find_nfeatures(hr1d, ones<bool>(hr1d.shape()), 2000);
    CorrespondingFeaturesOnLine ol{features2k, im0_rgb, im1_rgb, F};
    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        draw_epilines_from_F(F, bmp, Rgb24::green());
        highlight_features(features2k, bmp, 2, Rgb24::red());
        highlight_features(ol.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file(bmp_filename);
    }
    return ol;
}

/**
 * Cf. "CorrespondingFeaturesOnLine"
 */
void dense_reconstruction(
    const ParsedArgs& args,
    const CorrespondingFeaturesOnLine& ol,
    const Array<float>& im0,
    const Array<float>& im1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    const ProjectionToTR& ptr,
    const Array<float>& intrinsic_matrix)
{
    if (!only_features2k) {
        size_t search_length = 200;
        Array<float> disparity = compute_disparity_gray_single_pixel(im0, im1, F, search_length);
        if (!only_features2k) {
            Array<float> res = disparity / (2.f * search_length) + 0.5f;
            // res = box_filter(res, ArrayShape{5, 5}, NAN);
            // res = guided_filter(im1, res, ArrayShape{3, 3}, float(1e-1));
            PpmImage bmp = PpmImage::from_float_grayscale(res);
            bmp.save_to_file("disparity_gsp.bmp");
        }
        if (false) {
            float worst_error = 1.f;
            Array<float> disparity = compute_disparity_rgb_patch(im0_rgb, im1_rgb, F, search_length, worst_error);
            if (!only_features2k) {
                Array<float> res = disparity / (float(std::sqrt(2)) * 2.f * search_length) + 0.5f;
                PpmImage bmp = PpmImage::from_float_grayscale(clipped(res, 0.f, 1.f));
                bmp.save_to_file("disparity_rgbp.bmp");
            }
        }

        Array<float> condition_number;
        Array<float> x = reconstruct_disparity(
            disparity,
            F,
            ptr.R,
            ptr.t,
            intrinsic_matrix,
            &condition_number);
        PpmImage::from_float_grayscale(
            normalized_and_clipped(x[0], -1.f, 1.f)
            ).save_to_file(args.named_value("--recon-0"));
        PpmImage::from_float_grayscale(
            normalized_and_clipped(x[1], -1.f, 1.f)
            ).save_to_file(args.named_value("--recon-1"));
        PpmImage::from_float_grayscale(
            //normalized_and_clipped(z)
            normalized_and_clipped(x[2], 0.f, 1.f)
            //normalized_and_clipped(guided_filter(im1, z, ArrayShape{5, 5}, float(1e-2)), 0.3f, 0.6f)
            ).save_to_file(args.named_value("--recon-2"));
        x[0].save_txt_2d("dense-0.m");
        x[1].save_txt_2d("dense-1.m");
        x[2].save_txt_2d("dense-2.m");
        condition_number.save_txt_2d("cond.m");
    }

    {
        NormalizedProjection np{Array<float>{std::list<Array<float>>{
            homogenized_Nx3(ol.y0_2d),
            homogenized_Nx3(ol.y1_2d)}}};
        Array<float> condition_number;
        Array<float> x = initial_reconstruction(
            ptr.R,
            ptr.t,
            np.normalized_intrinsic_matrix(intrinsic_matrix),
            np.yn[0],
            np.yn[1],
            false, // points are normalized
            &condition_number);
        x.save_txt_2d("features2k.m");

        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
        cams.insert(std::make_pair(std::chrono::milliseconds{0}, CameraFrame{identity_array<float>(3), zeros<float>(ArrayShape{3}), CameraFrame::undefined_kep}));
        cams.insert(std::make_pair(std::chrono::milliseconds{5}, CameraFrame{ptr.R, ptr.t, CameraFrame::undefined_kep}));
        DenseProjector{cams, 0, 1, 2, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb}.normalize(256).draw("features2k-0-1.ppm");
        DenseProjector{cams, 0, 2, 1, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb}.normalize(256).draw("features2k-0-2.ppm");
        DenseProjector{cams, 2, 1, 0, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), im0_rgb}.normalize(256).draw("features2k-2-1.ppm");
    }
}

void compute_z(const ParsedArgs& args) {
    Bgr24Raw source0 = Bgr24Raw::load_from_file(args.unnamed_value(0));
    Bgr24Raw source1 = Bgr24Raw::load_from_file(args.unnamed_value(1));

    if (!all(source0.shape() == source1.shape())) {
        throw std::runtime_error("Source images differ in size");
    }

    Array<float> intrinsic_matrix = Array<float>::load_txt_2d(args.named_value("--calibration-filename"));
    if (!all(intrinsic_matrix.shape() == ArrayShape{3, 3})) {
        throw std::runtime_error("Intrinsic matrix must be 3x3");
    }

    Array<float> im0_rgb = source0.to_float_rgb();
    Array<float> im0 = source0.to_float_grayscale();
    Array<float> hr0 = harris_response(im0);
    Array<float> feature_points0 = find_nfeatures(
        -hr0,
        // find_local_maxima(-hr0, false),
        ones<bool>(im0.shape()),
        100);

    Array<float> im1_rgb = source1.to_float_rgb();
    Array<float> im1 = source1.to_float_grayscale();
    Array<float> hr1 = harris_response(im1);
    Array<float> feature_points2 = find_nfeatures(
        -hr1,
        // find_local_maxima(-hr1, false),
        ones<bool>(im1.shape()),
        100);

    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im0);
        highlight_features(feature_points0, bmp, 2);
        bmp.save_to_file("features0.bmp");
    }
    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        highlight_features(feature_points2, bmp, 2);
        bmp.save_to_file("features1.bmp");
    }

    CorrespondingFeaturesInBox cf{feature_points0, im0_rgb, im1_rgb};
    Array<float> y0{homogenized_Nx3(cf.y0_2d)};
    Array<float> y1{homogenized_Nx3(cf.y1_2d)};
    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
        highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file("features21.bmp");
    }

    Array<float> F = find_fundamental_matrix(y0, y1);
    std::cerr << "fundamental error " << sum(squared(fundamental_error(F, y0, y1))) << std::endl;
    std::cerr << "fundamental.T error " << sum(squared(fundamental_error(F.T(), y0, y1))) << std::endl;

    //Array<float> err = fundamental_error(F, y0, y1);
    //std::cerr << "err " << err << std::endl;
    //std::list<Array<float>> bad_points;
    //for (size_t r = 0; r < y0.shape(1); ++r) {
    //    if (std::abs(err(r)) > 1 * mean(abs(err))) {
    //        std::cerr << r << " " << y0[r] << std::endl;
    //        bad_points.push_back(y0_2[r]);
    //    }
    //}
    if (any(Mlib::isnan(F))) {
        throw std::runtime_error("Could not determine fundamental matrix");
    }
    if (false) {
        Array<float> epipole3 = find_epipole(F);
        if (any(Mlib::isnan(epipole3))) {
            throw std::runtime_error("Could not determine epipole");
        }
        Array<float> epipole2 = dehomogenized_2(epipole3 / epipole3(2));

        if (!only_features2k) {
            PpmImage bmp = PpmImage::from_float_grayscale(im1);
            draw_epilines_from_epipole(epipole2, bmp, Rgb24::green());
            highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
            highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
            //highlight_features(Array<float>(bad_points), bmp, 2, Bgr565::black());
            bmp.save_to_file("epilines.bmp");
        }
    }

    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        draw_epilines_from_F(F, bmp, Rgb24::green());
        highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
        highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file("f_epilines2.bmp");
    }

    if (!only_features2k) {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        draw_epilines_from_F(F.T(), bmp, Rgb24::green());
        highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
        highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file("f_epilines2_T.bmp");
    }

    CorrespondingFeaturesOnLine ol = get_cfol(im1, im0_rgb, im1_rgb, F, "f_hr1d.bmp");

    ProjectionToTrRansac ptr{
        y0,
        y1,
        intrinsic_matrix,
        0,
        RansacOptions<float> {
            nelems_small: 20,
            ncalls: 10,
            inlier_distance_thresh: squared(100.f),
            inlier_count_thresh: 10,
            seed: 1
        }};

    if (ptr.ptr == nullptr) {
        throw std::runtime_error("RANSAC found no candidate");
    } else {
        std::cerr << "ngood " << ptr.ptr->ngood << std::endl;
    }

    if (!ptr.ptr->good()) {
        throw std::runtime_error("Projection not good");
    }

    Array<float> F_r = fundamental_from_camera(
        intrinsic_matrix,
        intrinsic_matrix,
        ptr.ptr->R,
        ptr.ptr->t);
    {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        draw_epilines_from_F(F_r, bmp, Rgb24::green());
        highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
        highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file("r_epilines2.bmp");
    }

    {
        PpmImage bmp = PpmImage::from_float_grayscale(im1);
        draw_epilines_from_F(F_r.T(), bmp, Rgb24::green());
        highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
        highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
        bmp.save_to_file("r_epilines2_T.bmp");
    }

    CorrespondingFeaturesOnLine ol_r = get_cfol(im1, im0_rgb, im1_rgb, F_r, "r_hr1d.bmp");

    dense_reconstruction(
        args,
        args.has_named("--tr2f") ? ol_r : ol,
        im0,
        im1,
        im0_rgb,
        im1_rgb,
        args.has_named("--tr2f") ? F_r : F,
        *ptr.ptr,
        intrinsic_matrix);
}

void compute_bokeh(const ParsedArgs& args) {
    PpmImage bmp0 = PpmImage::load_from_file(args.unnamed_value(0));
    PpmImage bmpz = PpmImage::load_from_file(args.named_value("--recon-2"));
    Array<float> im0 = bmp0.to_float_grayscale();
    Array<float> z = bmpz.to_float_grayscale();
    Array<float> zf = clipped(guided_filter(im0, z, ArrayShape{15, 15}, float(1e-3)), 0.f, 1.f);
    PpmImage::from_float_grayscale(zf).save_to_file(args.named_value("--post"));

    Array<float> f;
    f = im0;
    for (float thr = 0.5; thr <= 1; thr += 0.1) {
        Array<float> f1 = box_filter_nan(f, ArrayShape{10, 10}, NAN);
        auto w = zf - thr;
        clip(w, 0.f, 1.f);
        f = f * (1.f - w) + f1 * w;
    }
    clip(f, 0.f, 1.f);
    PpmImage::from_float_grayscale(f).save_to_file(args.named_value("--bokeh"));
    /*for (size_t i = 0; i < 10; ++i) {

    }*/
}

void plot_dense_x(const ParsedArgs& args) {

    Array<float> intrinsic_matrix = Array<float>::load_txt_2d(args.named_value("--calibration-filename"));
    if (!all(intrinsic_matrix.shape() == ArrayShape{3, 3})) {
        throw std::runtime_error("Intrinsic matrix must be 3x3");
    }

    Bgr24Raw source0 = Bgr24Raw::load_from_file(args.unnamed_value(0));

    Array<float> x(std::list<Array<float>>{
        Array<float>::load_txt_2d("dense-0.m"),
        Array<float>::load_txt_2d("dense-1.m"),
        Array<float>::load_txt_2d("dense-2.m")});
    Array<float> condition_number = Array<float>::load_txt_2d("cond.m");

    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
    DenseProjector::from_image(cams, 0, 2, 1, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), source0.to_float_rgb()).normalize(256).draw("dense-0-2.ppm");
    DenseProjector::from_image(cams, 2, 1, 0, x, condition_number, intrinsic_matrix, dehomogenized_3x4(identity_array<float>(4)), source0.to_float_rgb()).normalize(256).draw("dense-2-1.ppm");
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: sfm_bokeh source0 source1 --recon <reconstructed> --post <postprocessed> --bokeh <postprocessed> --calibration-filename <calibration-filename> [--tr2f]",
        {"--tr2f"},
        {"--recon-0", "--recon-1", "--recon-2", "--post", "--bokeh", "--calibration-filename"});
    const auto args = parser.parsed(argc, argv);

    args.assert_num_unamed(2);

    if (!fs::exists(args.named_value("--recon-2"))) {
        std::cout << "File " << args.named_value("--recon-2") << " does not exist, recomputing..." << std::endl;
        compute_z(args);
    } else {
        std::cout << "File " << args.named_value("--recon-2") << " already exists." << std::endl;
    }
    compute_bokeh(args);
    plot_dense_x(args);
    return 0;
}
