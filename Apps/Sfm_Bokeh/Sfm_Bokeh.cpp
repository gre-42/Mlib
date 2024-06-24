#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Coordinates/Base_Projector.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Guided_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_In_Box.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_In_Candidate_List.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Features_On_Line.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Draw/Sparse_Projector.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
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
    const FixedArray<float, 3, 3>& F,
    const std::string& bmp_filename)
{
    Array<float> hr1d = harris_response_1d(im1, F);
    Array<FixedArray<float, 2>> features2k = Array<float>::from_dynamic<2>(find_nfeatures(hr1d, ones<bool>(hr1d.shape()), 2000, 2.f));
    CorrespondingFeaturesOnLine ol{features2k, im0_rgb, im1_rgb, F};
    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
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
    const FixedArray<float, 3, 3>& F,
    const ProjectionToTR& ptr,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix)
{
    if (!only_features2k) {
        size_t search_length = 200;
        Array<float> disparity = compute_disparity_rgb_single_pixel(im0_rgb, im1_rgb, F, search_length);
        if (!only_features2k) {
            Array<float> res = disparity / (2.f * search_length) + 0.5f;
            // res = box_filter(res, ArrayShape{5, 5}, NAN);
            // res = guided_filter(im1, res, ArrayShape{3, 3}, float(1e-1));
            StbImage3 bmp = StbImage3::from_float_grayscale(res);
            bmp.save_to_file("disparity_rgbs.png");
        }
        if (false) {
            float worst_error = 1.f;
            Array<float> disparity = compute_disparity_rgb_patch(im0_rgb, im1_rgb, F, search_length, worst_error);
            if (!only_features2k) {
                Array<float> res = disparity / (float(std::sqrt(2)) * 2.f * search_length) + 0.5f;
                StbImage3 bmp = StbImage3::from_float_grayscale(clipped(res, 0.f, 1.f));
                bmp.save_to_file("disparity_rgbp.png");
            }
        }

        Array<float> condition_number;
        Array<float> x = reconstruct_disparity(
            disparity,
            F,
            intrinsic_matrix,
            ptr.ke,
            &condition_number);
        StbImage3::from_float_grayscale(
            normalized_and_clipped(x[0], -1.f, 1.f)
            ).save_to_file(args.named_value("--recon-0"));
        StbImage3::from_float_grayscale(
            normalized_and_clipped(x[1], -1.f, 1.f)
            ).save_to_file(args.named_value("--recon-1"));
        StbImage3::from_float_grayscale(
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
        NormalizedProjection np{Array<FixedArray<float, 2>>{std::list<Array<FixedArray<float, 2>>>{
            ol.y0_2d,
            ol.y1_2d}}};
        Array<float> condition_number;
        Array<FixedArray<float, 3>> x = initial_reconstruction(
            ptr.ke,
            np.normalized_intrinsic_matrix(intrinsic_matrix),
            np.yn[0],
            np.yn[1],
            false, // points are normalized
            &condition_number);
        x.save_txt_2d("features2k.m");

        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
        cams.insert(std::make_pair(std::chrono::milliseconds{0}, CameraFrame{ TransformationMatrix<float, float, 3>::identity() }));
        cams.insert(std::make_pair(std::chrono::milliseconds{5}, CameraFrame{ ptr.ke.inverted() }));
        DenseProjector{cams, 0, 1, 2, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb}.normalize(256).draw("features2k-0-1.png");
        DenseProjector{cams, 0, 2, 1, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb}.normalize(256).draw("features2k-0-2.png");
        DenseProjector{cams, 2, 1, 0, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb}.normalize(256).draw("features2k-2-1.png");
    }
}

void compute_z(const ParsedArgs& args) {
    StbImage3 source0 = StbImage3::load_from_file(args.unnamed_value(0));
    StbImage3 source1 = StbImage3::load_from_file(args.unnamed_value(1));

    if (!all(source0.shape() == source1.shape())) {
        throw std::runtime_error("Source images differ in size");
    }

    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{Array<float>::load_txt_2d(args.named_value("--calibration-filename"))} };

    Array<float> im0_rgb = source0.to_float_rgb();
    Array<float> im0 = source0.to_float_grayscale();
    Array<float> hr0 = harris_response(im0);
    Array<FixedArray<float, 2>> feature_points0 = Array<float>::from_dynamic<2>(find_nfeatures(
        hr0,
        // find_local_maxima(-hr0, false),
        ones<bool>(im0.shape()),
        100,
        2.f));

    Array<float> im1_rgb = source1.to_float_rgb();
    Array<float> im1 = source1.to_float_grayscale();
    Array<float> hr1 = harris_response(im1);
    Array<FixedArray<float, 2>> feature_points1 = Array<float>::from_dynamic<2>(find_nfeatures(
        hr1,
        // find_local_maxima(-hr1, false),
        ones<bool>(im1.shape()),
        100,
        2.f));

    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im0);
        highlight_features(feature_points0, bmp, 2);
        bmp.save_to_file("features0.png");
    }
    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        highlight_features(feature_points1, bmp, 2);
        bmp.save_to_file("features1.png");
    }

    // CorrespondingFeaturesInBox cf{feature_points0, im0_rgb, im1_rgb, 10, 30};
    CorrespondingFeaturesInCandidateList cf{feature_points0, feature_points1, im0_rgb, im1_rgb, 10};
    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im0);
        highlight_feature_correspondences(cf.y0, cf.y1, bmp);
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("features10_0.png");
    }
    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        highlight_feature_correspondences(cf.y0, cf.y1, bmp);
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("features10_1.png");
    }

    FixedArray<float, 3, 3> F = find_fundamental_matrix(cf.y0, cf.y1);
    lerr() << "fundamental error " << sum(squared(fundamental_error(F, cf.y0, cf.y1)));
    lerr() << "fundamental.T error " << sum(squared(fundamental_error(F.T(), cf.y0, cf.y1)));

    //Array<float> err = fundamental_error(F, y0, y1);
    //lerr() << "err " << err;
    //std::list<Array<float>> bad_points;
    //for (size_t r = 0; r < y0.shape(1); ++r) {
    //    if (std::abs(err(r)) > 1 * mean(abs(err))) {
    //        lerr() << r << " " << y0[r];
    //        bad_points.push_back(y0_2[r]);
    //    }
    //}
    if (any(Mlib::isnan(F))) {
        throw std::runtime_error("Could not determine fundamental matrix");
    }
    if (false) {
        FixedArray<float, 2> epipole2 = find_epipole(F);
        if (any(Mlib::isnan(epipole2))) {
            throw std::runtime_error("Could not determine epipole");
        }

        if (!only_features2k) {
            StbImage3 bmp = StbImage3::from_float_grayscale(im1);
            draw_epilines_from_epipole(epipole2, bmp, Rgb24::green());
            highlight_features(cf.y0, bmp, 2, Rgb24::red());
            highlight_features(cf.y1, bmp, 2, Rgb24::blue());
            //highlight_features(Array<float>(bad_points), bmp, 2, Bgr565::black());
            bmp.save_to_file("epilines.png");
        }
    }

    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        draw_epilines_from_F(F, bmp, Rgb24::green());
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("f_epilines2.png");
    }

    if (!only_features2k) {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        draw_epilines_from_F(F.T(), bmp, Rgb24::green());
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("f_epilines2_T.png");
    }

    CorrespondingFeaturesOnLine ol = get_cfol(im1, im0_rgb, im1_rgb, F, "f_hr1d.png");

    ProjectionToTrRansac ptr{
        cf.y0,
        cf.y1,
        intrinsic_matrix,
        { 0.f, float{INFINITY} },
        RansacOptions<float> {
            .nelems_small = 8,
            .ncalls = 100,
            .inlier_distance_thresh = squared(2.f),
            .inlier_count_thresh = 20,
            .seed = 1
        }};

    if (ptr.ptr == nullptr) {
        throw std::runtime_error("RANSAC found no candidate");
    } else {
        lerr() << "ngood " << ptr.ptr->ngood;
    }

    if (!ptr.ptr->good()) {
        throw std::runtime_error("Projection not good");
    }

    {
        auto sparse_reconstruction = ptr.ptr->initial_reconstruction().reconstructed();
        MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>> reconstructed_points;
        for (size_t i = 0; i < sparse_reconstruction.length(); ++i) {
            reconstructed_points.active_[i] = std::make_shared<ReconstructedPoint>(sparse_reconstruction(i), NAN);
        }
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> camera_frames;
        camera_frames.active_.insert({std::chrono::milliseconds{ 0 }, CameraFrame{ TransformationMatrix<float, float, 3>::identity() }});
        camera_frames.active_.insert({std::chrono::milliseconds{ 42 }, CameraFrame{ ptr.ptr->ke.inverted() }});
        SparseProjector(reconstructed_points, {}, camera_frames, 0, 1, 2).normalize(256).draw("features-0-1.png");
        SparseProjector(reconstructed_points, {}, camera_frames, 0, 2, 1).normalize(256).draw("features-0-2.png");
        SparseProjector(reconstructed_points, {}, camera_frames, 2, 1, 0).normalize(256).draw("features-2-1.png");
    }

    FixedArray<float, 3, 3> F_r = fundamental_from_camera(
        intrinsic_matrix,
        intrinsic_matrix,
        ptr.ptr->ke.inverted());
    {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        draw_epilines_from_F(F_r, bmp, Rgb24::green());
        highlight_feature_correspondences(cf.y0, cf.y1, bmp);
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("r_epilines2.png");
    }

    {
        StbImage3 bmp = StbImage3::from_float_grayscale(im1);
        draw_epilines_from_F(F_r.T(), bmp, Rgb24::green());
        highlight_feature_correspondences(cf.y0, cf.y1, bmp);
        highlight_features(cf.y0, bmp, 2, Rgb24::red());
        highlight_features(cf.y1, bmp, 2, Rgb24::blue());
        bmp.save_to_file("r_epilines2_T.png");
    }

    CorrespondingFeaturesOnLine ol_r = get_cfol(im1, im0_rgb, im1_rgb, F_r, "r_hr1d.png");

    dense_reconstruction(
        args,
        args.has_named("--no_tr2f") ? ol : ol_r,
        im0,
        im1,
        im0_rgb,
        im1_rgb,
        args.has_named("--no_tr2f") ? F : F_r,
        *ptr.ptr,
        intrinsic_matrix);
}

void compute_bokeh(const ParsedArgs& args) {
    StbImage3 bmp0 = StbImage3::load_from_file(args.unnamed_value(0));
    StbImage3 bmpz = StbImage3::load_from_file(args.named_value("--recon-2"));
    Array<float> im0 = bmp0.to_float_grayscale();
    Array<float> z = bmpz.to_float_grayscale();
    Array<float> zf = clipped(guided_filter(im0, z, ArrayShape{15, 15}, float(1e-3)), 0.f, 1.f);
    StbImage3::from_float_grayscale(zf).save_to_file(args.named_value("--post"));

    Array<float> f;
    f = im0;
    for (float thr = 0.5; thr <= 1.f; thr += 0.1f) {
        Array<float> f1 = box_filter_nan(f, ArrayShape{10, 10}, NAN);
        auto w = zf - thr;
        clip(w, 0.f, 1.f);
        f = f * (1.f - w) + f1 * w;
    }
    clip(f, 0.f, 1.f);
    StbImage3::from_float_grayscale(f).save_to_file(args.named_value("--bokeh"));
    /*for (size_t i = 0; i < 10; ++i) {

    }*/
}

void plot_dense_x(const ParsedArgs& args) {

    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{Array<float>::load_txt_2d(args.named_value("--calibration-filename"))} };

    StbImage3 source0 = StbImage3::load_from_file(args.unnamed_value(0));

    Array<float> x(std::list<Array<float>>{
        Array<float>::load_txt_2d("dense-0.m"),
        Array<float>::load_txt_2d("dense-1.m"),
        Array<float>::load_txt_2d("dense-2.m")});
    Array<float> condition_number = Array<float>::load_txt_2d("cond.m");

    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
    DenseProjector::from_image(cams, 0, 2, 1, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), source0.to_float_rgb()).normalize(256).draw("dense-0-2.png");
    DenseProjector::from_image(cams, 2, 1, 0, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), source0.to_float_rgb()).normalize(256).draw("dense-2-1.png");
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: sfm_bokeh source0 source1 --recon <reconstructed> --post <postprocessed> --bokeh <postprocessed> --calibration-filename <calibration-filename> [--no_tr2f]",
        {"--no_tr2f"},
        {"--recon-0", "--recon-1", "--recon-2", "--post", "--bokeh", "--calibration-filename"});
    const auto args = parser.parsed(argc, argv);

    args.assert_num_unnamed(2);

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
