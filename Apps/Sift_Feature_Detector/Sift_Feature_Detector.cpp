#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/CvSift/CvSift.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Sift.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sift;

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
          "--clip-min",
          "--clip-max" });
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unamed(2);
        auto bitmap0 = StbImage::load_from_file(args.unnamed_value(0));
        {
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
        {
            cv::SIFT sift{ 200 };
            std::vector<cv::KeyPoint> keypoints;
            sift(bitmap0.to_float_grayscale().applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);}), Array<uint8_t>(), keypoints);
            Array<FixedArray<float, 2>> corners0 = Array(std::list(keypoints.begin(), keypoints.end()))
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
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
