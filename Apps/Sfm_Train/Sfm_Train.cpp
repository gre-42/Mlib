#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Sfm/Configuration/Regularization.hpp>
#include <Mlib/Sfm/Configuration/Tracking_Mode.hpp>
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
        process_folder_with_pipeline(cache_dir, calib_source_dir, nullptr, *calib, std::cout, 0, SIZE_MAX, SIZE_MAX, false);
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
    bool reverse,
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
        ncameras,
        reverse);
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
        "[--sift] "
        "[--sift_nframes <nframes>] "
        "[--use_virtual_camera] "
        "--chess_r <chess_r> "
        "--chess_c <chess_c> "
        "[--nskipped <nskipped>] "
        "[--nimages <nimages>] "
        "[--ncameras <ncameras>] "
        "[--reverse] "
        "[--features_down_sampling <n>] "
        "[--dtam_down_sampling <n>] "
        "[--cost_volume {multichannel_flat,multichannel_pyramid}] "
        "[--regularization {dtam,dense_geometry,dense_geometry_pyramid,filter}] "
        "[--regularization_filter_poly_degree <d>] "
        "[--regularization_filter_sigma <s>] "
        "[--optimize_parameters]",
        { "--no_dtam",
          "--no_dtam_tracking",
          "--sift",
          "--use_virtual_camera",
          "--reverse",
          "--optimize_parameters" },
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
          "--sift_nframes",
          "--features_down_sampling",
          "--dtam_down_sampling",
          "--cost_volume",
          "--regularization",
          "--regularization_filter_sigma",
          "--regularization_filter_poly_degree" });

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
            args.has_named("--reverse"),
            TemplatePatchPipelineConfig{
                .tracking_mode = args.has_named("--sift") ? TrackingMode::SIFT : TrackingMode::PATCH_NEW_POSITION_IN_BOX,
                .cost_volume_type = cost_volume_type_from_string(args.named_value("--cost_volume", "multichannel_flat")),
                .regularization = regularization_from_string(args.named_value("--regularization", "dense_geometry")),
                .sift_nframes = safe_stoz(args.named_value("--sift_nframes", "19")),
                .enable_dtam = !args.has_named("--no_dtam"),
                .track_using_dtam = !args.has_named("--no_dtam_tracking"),
                .use_virtual_camera = args.has_named("--use_virtual_camera"),
                .print_residual = args.has_named("--print_residual"),
                .features_down_sampling = safe_stoz(args.named_value("--features_down_sampling", "0")),
                .dtam_down_sampling = safe_stoz(args.named_value("--dtam_down_sampling", "0")),
                .regularization_filter_sigma = safe_stof(args.named_value("--regularization_filter_sigma", "1")),
                .regularization_filter_poly_degree = safe_stoz(args.named_value("--regularization_filter_poly_degree", "0")),
                .optimize_parameters = args.has_named("--optimize_parameters") });
        return 0;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
