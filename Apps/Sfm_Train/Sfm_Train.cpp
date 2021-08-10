#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Sfm/Data_Generators/Folder_Data_Generator.hpp>
#include <Mlib/Sfm/Pipelines/Calibration/Chessboard_Calibration_Pipeline.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;

static std::unique_ptr<ChessboardCalibrationPipeline> run_chessboard_calibration_pipeline(
    const std::string& calib_source_dir,
    const std::string& cache_dir,
    const ArrayShape& chessboard_shape)
{
    auto calib = std::make_unique<ChessboardCalibrationPipeline>(
        cache_dir,
        chessboard_shape);
    if (!calib->is_cached()) {
        process_folder_with_pipeline(cache_dir, calib_source_dir, nullptr, *calib, std::cout, 0, SIZE_MAX, SIZE_MAX);
    }
    return calib;
}

static void run_reconstruction_pipeline(
    const std::string& cache_dir,
    const std::string& calib_source_dir,
    const std::string& recon_source_dir,
    const std::string* camera_source_dir,
    const ArrayShape& chessboard_shape,
    size_t nskipped,
    size_t nimages,
    size_t ncameras,
    const TemplatePatchPipelineConfig& cfg)
{
    std::unique_ptr<ImagePipeline> pipeline;
    {
        auto calibration = run_chessboard_calibration_pipeline(
            calib_source_dir,
            cache_dir,
            chessboard_shape);
        pipeline = std::unique_ptr<ImagePipeline>(new TemplatePatchPipeline(
            cache_dir,
            calibration->intrinsic_matrix(),
            cfg));
    }
    process_folder_with_pipeline(
        cache_dir,
        recon_source_dir,
        camera_source_dir,
        *pipeline,
        std::cout,
        nskipped,
        nimages,
        ncameras);
}


int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: --cache <cache_dir> "
        "--calib_source <source_dir> "
        "--recon_source <source_dir> "
        "--camera_source <source_dir> "
        "[--no_dtam] "
        "[--no_dtam_tracking] "
        "--chess_r <chess_r> "
        "--chess_c <chess_c> "
        "[--nskipped <nskipped>] "
        "[--nimages <nimages>] "
        "[--ncameras <ncameras>] "
        "[--dtam_down_sampling <n>]",
        { "--no_dtam",
          "--no_dtam_tracking" },
        { "--pipeline",
          "--cache",
          "--calib_source",
          "--recon_source",
          "--camera_source",
          "--chess_r",
          "--chess_c",
          "--nskipped",
          "--nimages",
          "--ncameras",
          "--dtam_down_sampling" });

    try {
        auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        const std::string cache_dir = args.named_value("--cache");
        const std::string calib_source_dir = args.named_value("--calib_source");
        const std::string recon_source_dir = args.named_value("--recon_source");
        const std::string camera_source_dir = args.named_value("--camera_source", "");
        ArrayShape chessboard_shape({
            safe_stoz(args.named_value("--chess_r")),
            safe_stoz(args.named_value("--chess_c")) });
        run_reconstruction_pipeline(
            cache_dir,
            calib_source_dir,
            recon_source_dir,
            args.has_named_value("--camera_source") ? &camera_source_dir : nullptr,
            chessboard_shape,
            args.has_named_value("--nskipped") ? safe_stoz(args.named_value("--nskipped")) : 0,
            args.has_named_value("--nimages") ? safe_stoz(args.named_value("--nimages")) : SIZE_MAX,
            args.has_named_value("--ncameras") ? safe_stoz(args.named_value("--ncameras")) : SIZE_MAX,
            TemplatePatchPipelineConfig{
                .enable_dtam = !args.has_named("--no_dtam"),
                .track_using_dtam = !args.has_named("--no_dtam_tracking"),
                .print_residual = args.has_named("--print_residual"),
                .dtam_down_sampling = safe_stoz(args.named_value("--dtam_down_sampling", "0")) });
        return 0;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
