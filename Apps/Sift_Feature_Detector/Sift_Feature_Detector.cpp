#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/CvSift/CvSift.hpp>
#include <Mlib/Images/CvSift/CvSift2.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/OpenCV.hpp>
#include <Mlib/Images/Sift.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Descriptors_In_Candidate_List.hpp>
#include <Mlib/Sfm/Draw/Sparse_Projector.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR_Ransac.hpp>
#include <Mlib/Stats/RansacOptions.hpp>
#include <Mlib/Strings/To_Number.hpp>

#ifndef WITHOUT_OPENCV
#include <opencv2/features2d.hpp>
#endif

using namespace Mlib;
using namespace Mlib::Sift;
using namespace Mlib::Sfm;

enum class SiftImpl {
    ONE,
    TWO,
    CV
};

SiftImpl parse_sift_impl(const std::string& impl_str) {
    if (impl_str == "1") {
        return SiftImpl::ONE;
    } else if (impl_str == "2") {
        return SiftImpl::TWO;
    } else if (impl_str == "cv") {
        return SiftImpl::CV;
    } else {
        throw std::runtime_error("Unknown SIFT implementation");
    }
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();
    const ArgParser parser(
        "Usage: sift_feature_detector "
        "source "
        "destination "
        "ncorners "
        "[--source1 <source1>] "
        "[--k <k>] "
        "[--distance-sigma <distance-sigma>] "
        "[--multi-scale] "
        "[--size <marker-size>] "
        "[--response <response>] "
        "[--clip-min <clip-min>] "
        "[--clip-max <clip-max>] "
        "[--impl {1,2,cv}] "
        "[--intrinsic_matrix <ki>]",
        { "--multi-scale" },
        { "--source1",
          "--impl",
          "--k",
          "--distance-sigma",
          "--size",
          "--response",
          "--response1",
          "--clip-min",
          "--clip-max",
          "--intrinsic_matrix" });
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(3);
        auto bitmap0 = StbImage3::load_from_file(args.unnamed_value(0));
        if (false) {
            SiftFeatures response0 = computeKeypointsAndDescriptors(bitmap0.to_float_grayscale());
            Array<FixedArray<float, 2>> corners0 = Array<KeyPointWithOrientation>(response0.keypoints).applied<FixedArray<float, 2>>([](const KeyPointWithOrientation& v){return v.kp.pt;});
            {
                StbImage3 bmp{bitmap0.copy()};
                highlight_features(
                    corners0,
                    bmp,
                    safe_stoi(args.named_value("--size", "1")));
                bmp.save_to_file(args.unnamed_value(1));
            }
        }
        Array<float> descriptors0;
        Array<FixedArray<float, 2>> corners0;
        SiftImpl impl = parse_sift_impl(args.named_value("--impl", "cv"));
        {
            if (impl == SiftImpl::ONE) {
                ocv::SIFT sift{ safe_stoi(args.unnamed_value(2)) };
                std::vector<ocv::KeyPoint> keypoints;
                sift(bitmap0.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors0);
                corners0 = Array<ocv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const ocv::KeyPoint& v){return v.pt;});
            } else if (impl == SiftImpl::TWO) {
                ocv::SIFT2 sift{ safe_stoi(args.unnamed_value(2)) };
                std::vector<ocv::KeyPoint> keypoints;
                sift.detectAndCompute(bitmap0.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors0);
                corners0 = Array<ocv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const ocv::KeyPoint& v){return v.pt;});
            } else {
#ifndef WITHOUT_OPENCV
                std::vector<cv::KeyPoint> keypoints;
                auto sift = cv::SIFT::create(safe_stoi(args.unnamed_value(2)));
                cv::Mat_<float> cv_descriptors0;
                sift->detectAndCompute(
                    array_to_cv_mat(bitmap0.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);})),
                    cv::Mat_<uint8_t>(),
                    keypoints,
                    cv_descriptors0);
                descriptors0 = cv_mat_to_array(cv_descriptors0);
                corners0 = Array<cv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const cv::KeyPoint& v){return FixedArray<float, 2>{ v.pt.x, v.pt.y };});
#else
                throw std::runtime_error("Compiled without OpenCV");
#endif
            }
            {
                StbImage3 bmp{bitmap0.copy()};
                highlight_features(
                    corners0,
                    bmp,
                    safe_stoi(args.named_value("--size", "1")));
                bmp.save_to_file(args.unnamed_value(1));
            }
        }
        Array<float> descriptors1;
        Array<FixedArray<float, 2>> corners1;
        if (args.has_named_value("--source1")) {
            auto bitmap1 = StbImage3::load_from_file(args.named_value("--source1"));
            if (impl == SiftImpl::ONE) {
                ocv::SIFT sift{ safe_stoi(args.unnamed_value(2)) };
                std::vector<ocv::KeyPoint> keypoints;
                sift(bitmap1.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors1);
                corners1 = Array<ocv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const ocv::KeyPoint& v){return v.pt;});
                if (args.has_named_value("--response1")) {
                    StbImage3 bmp{bitmap1.copy()};
                    highlight_features(
                        corners1,
                        bmp,
                        safe_stoi(args.named_value("--size", "1")));
                    bmp.save_to_file(args.named_value("--response1"));
                }
            } else if (impl == SiftImpl::TWO) {
                ocv::SIFT2 sift{ safe_stoi(args.unnamed_value(2)) };
                std::vector<ocv::KeyPoint> keypoints;
                sift.detectAndCompute(bitmap1.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors1);
                corners1 = Array<ocv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const ocv::KeyPoint& v){return v.pt;});
                if (args.has_named_value("--response1")) {
                    StbImage3 bmp{bitmap1.copy()};
                    highlight_features(
                        corners1,
                        bmp,
                        safe_stoi(args.named_value("--size", "1")));
                    bmp.save_to_file(args.named_value("--response1"));
                }
            } else {
#ifndef WITHOUT_OPENCV
                std::vector<cv::KeyPoint> keypoints;
                auto sift = cv::SIFT::create(safe_stoi(args.unnamed_value(2)));
                cv::Mat_<float> cv_descriptors1;
                sift->detectAndCompute(
                    array_to_cv_mat(bitmap1.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);})),
                    cv::Mat_<uint8_t>(),
                    keypoints,
                    cv_descriptors1);
                descriptors1 = cv_mat_to_array(cv_descriptors1);
                corners1 = Array<cv::KeyPoint>(keypoints)
                    .applied<FixedArray<float, 2>>([](const cv::KeyPoint& v){return FixedArray<float, 2>{ v.pt.x, v.pt.y };});
#else
                throw std::runtime_error("Compiled without OpenCV");
#endif
            }
            CorrespondingDescriptorsInCandidateList cf{corners0, corners1, descriptors0, descriptors1};
            {
                StbImage3 bmp = StbImage3::from_float_rgb((bitmap0.to_float_rgb() + bitmap1.to_float_rgb()) / 2.f);
                highlight_features(cf.y0, bmp, 2, Rgb24::red());
                highlight_features(cf.y1, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0, cf.y1, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features01.png");
            }
            {
                StbImage3 bmp{ bitmap0.copy() };
                highlight_features(cf.y0, bmp, 2, Rgb24::red());
                highlight_features(cf.y1, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0, cf.y1, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features01_0.png");
            }
            {
                StbImage3 bmp{ bitmap1.copy() };
                highlight_features(cf.y0, bmp, 2, Rgb24::red());
                highlight_features(cf.y1, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0, cf.y1, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features01_1.png");
            }
            if (args.has_named_value("--intrinsic_matrix")) {
                TransformationMatrix<float, float, 2> ki { FixedArray<float, 3, 3>{ Array<float>::load_txt_2d(args.named_value("--intrinsic_matrix")) } };
                ProjectionToTrRansac ptr{
                    cf.y0,
                    cf.y1,
                    ki,
                    { 1.f, float{INFINITY} },
                    RansacOptions<float>{
                        .nelems_small = 8,
                        .ncalls = 100,
                        .inlier_distance_thresh = squared(4.f),
                        .inlier_count_thresh = 20,
                        .seed = 1}};
                if (ptr.ptr == nullptr) {
                    lerr() << "RANSAC found no candidate";
                } else {
                    lerr() << ptr.ptr->ke.semi_affine();

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
            }
        }
    } catch (const std::runtime_error& e) {
        lerr() << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
