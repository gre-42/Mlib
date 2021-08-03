#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/CvSift/CvSift.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Sift.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Descriptors_In_Candidate_List.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sift;
using namespace Mlib::Sfm;

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
        "[--clip-max <clip-max>]",
        { "--multi-scale" },
        { "--source1",
          "--k",
          "--distance-sigma",
          "--size",
          "--response",
          "--response1",
          "--clip-min",
          "--clip-max" });
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unamed(3);
        auto bitmap0 = StbImage::load_from_file(args.unnamed_value(0));
        if (false) {
            SiftFeatures response0 = computeKeypointsAndDescriptors(bitmap0.to_float_grayscale());
            Array<FixedArray<float, 2>> corners0 = Array(response0.keypoints).applied<FixedArray<float, 2>>([](const KeyPointWithOrientation& v){return v.kp.pt;});
            {
                StbImage bmp{bitmap0.copy()};
                highlight_features(
                    corners0,
                    bmp,
                    safe_stoi(args.named_value("--size", "1")));
                bmp.save_to_file(args.unnamed_value(1));
            }
        }
        Array<float> descriptors0;
        Array<FixedArray<float, 2>> corners0;
        {
            cv::SIFT sift{ safe_stoi(args.unnamed_value(2)) };
            std::vector<cv::KeyPoint> keypoints;
            sift(bitmap0.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors0);
            corners0 = Array(std::list(keypoints.begin(), keypoints.end()))
                .applied<FixedArray<float, 2>>([](const cv::KeyPoint& v){return v.pt;});
            {
                StbImage bmp{bitmap0.copy()};
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
            auto bitmap1 = StbImage::load_from_file(args.named_value("--source1"));
            cv::SIFT sift{ safe_stoi(args.unnamed_value(2)) };
            std::vector<cv::KeyPoint> keypoints;
            sift(bitmap1.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints, &descriptors1);
            corners1 = Array(std::list(keypoints.begin(), keypoints.end()))
                .applied<FixedArray<float, 2>>([](const cv::KeyPoint& v){return v.pt;});
            {
                StbImage bmp{bitmap1.copy()};
                highlight_features(
                    corners1,
                    bmp,
                    safe_stoi(args.named_value("--size", "1")));
                bmp.save_to_file(args.named_value("--response1"));
            }
            CorrespondingDescriptorsInCandidateList cf{corners0, corners1, descriptors0, descriptors1};
            {
                StbImage bmp{ bitmap0.copy() };
                highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
                highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0_2d, cf.y1_2d, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features10_0.png");
            }
            {
                StbImage bmp{ bitmap1.copy() };
                highlight_features(cf.y0_2d, bmp, 2, Rgb24::red());
                highlight_features(cf.y1_2d, bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(cf.y0_2d, cf.y1_2d, bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                bmp.save_to_file("features10_1.png");
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
