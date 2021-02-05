#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Sfm/Data_Generators/Folder_Data_Generator.hpp>
#include <Mlib/Sfm/Pipelines/Calibration/Chessboard_Calibration_Pipeline.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <fenv.h>

using namespace Mlib;
using namespace Mlib::Sfm;

static std::unique_ptr<ChessboardCalibrationPipeline> run_chessboard_calibration_pipeline(
    const std::string& source_dir,
    const std::string& cache_dir,
    const ArrayShape& chessboard_shape)
{
    const std::string calibration_dir = source_dir + "/0-calibration/ppm";

    auto calib = std::make_unique<ChessboardCalibrationPipeline>(
        cache_dir,
        chessboard_shape);
    if (!calib->is_cached()) {
        process_folder_with_pipeline(cache_dir, calibration_dir, nullptr, *calib, std::cout, SIZE_MAX, SIZE_MAX);
    }
    return calib;
}

static void run_reconstruction_pipeline(
    const std::string& sfm_pipeline,
    const std::string& cache_dir,
    const std::string& source_dir,
    bool load_cameras,
    const ArrayShape& chessboard_shape,
    size_t nimages,
    size_t ncameras,
    const TemplatePatchPipelineConfig& cfg)
{
    std::unique_ptr<ImagePipeline> pipeline;
    if (sfm_pipeline == "optical_flow") {
        auto calibration = run_chessboard_calibration_pipeline(
            source_dir,
            cache_dir,
            chessboard_shape);
        pipeline = std::unique_ptr<ImagePipeline>(new TemplatePatchPipeline(
            cache_dir,
            calibration->intrinsic_matrix(),
            cfg));
    } else {
        throw std::runtime_error("Unknown pipeline: " + sfm_pipeline);
    }
    std::string camera_dir = source_dir + "/1-video/cameras";
    process_folder_with_pipeline(cache_dir, source_dir + "/1-video/ppm", load_cameras ? &camera_dir : nullptr, *pipeline, std::cout, nimages, ncameras);
}

#ifdef _MSC_VER
#pragma float_control(except, on)
#endif

int main(int argc, char **argv) {
    #ifdef __linux__
    feenableexcept(FE_INVALID);
    #endif

    ArgParser parser(
        "Usage: [--pipeline {feature_points, optical_flow}] --cache <cache_dir> --source <source_dir> [--load_cameras] [--no-dtam] [--no-dtam-tracking] --chess_r <chess_r> --chess_c <chess_c> [--nimages <nimages>] [--ncameras <ncameras>]",
        {"--load_cameras", "--no-dtam", "--no-dtam-tracking"},
        {"--pipeline", "--cache", "--source", "--chess_r", "--chess_c", "--nimages", "--ncameras"});

    try {
        auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        const std::string sfm_pipeline = args.named_value("--pipeline", "feature_points");
        const std::string cache_dir = args.named_value("--cache");
        const std::string image_dir = args.named_value("--source");
        ArrayShape chessboard_shape({
            safe_stoz(args.named_value("--chess_r")),
            safe_stoz(args.named_value("--chess_c"))});
        run_reconstruction_pipeline(
            sfm_pipeline,
            cache_dir,
            image_dir,
            args.has_named("--load_cameras"),
            chessboard_shape,
            args.has_named_value("--nimages") ? safe_stoz(args.named_value("--nimages")) : SIZE_MAX,
            args.has_named_value("--ncameras") ? safe_stoz(args.named_value("--ncameras")) : SIZE_MAX,
            TemplatePatchPipelineConfig{
                .enable_dtam = !args.has_named("--no-dtam"),
                .track_using_dtam = !args.has_named("--no-dtam-tracking")});
        return 0;
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
