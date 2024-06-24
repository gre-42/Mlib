#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Sfm/Configuration/Regularization.hpp>
#include <Mlib/Sfm/Configuration/Tracking_Mode.hpp>
#include <Mlib/Sfm/Data_Generators/Folder_Data_Generator.hpp>
#include <Mlib/Sfm/Pipelines/Calibration/Chessboard_Calibration_Pipeline.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline.hpp>
#include <Mlib/Strings/To_Number.hpp>
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
        pipeline = std::make_unique<TemplatePatchPipeline>(
            cache_dir,
            rotated_intrinsic_matrix(calibration->intrinsic_matrix(), calibration->sensor_size(), cfg.calibration_rotations),
            cfg);
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
        "[--tracking_mode {patches,cv_sift,cv_sift_0,cv_sift_1}] "
        "[--sift_nframes <nframes>] "
        "[--fov_min <value>] "
        "[--fov_max <value>] "
        "[--use_virtual_camera] "
        "--chess_r <chess_r> "
        "--chess_c <chess_c> "
        "[--calibration_rotations <n>] "
        "[--epipole_radius <r>] "
        "[--nskipped <nskipped>] "
        "[--nimages <nimages>] "
        "[--ncameras <ncameras>] "
        "[--reverse] "
        "[--features_down_sampling <n>] "
        "[--dtam_down_sampling <n>] "
        "[--regularization {dtam,dense_geometry,dense_geometry_pyramid,filter}] "
        "[--regularization_filter_poly_degree <d>] "
        "[--regularization_filter_sigma <s>] "
        "[--regularization_lambda <lambda> ]"
        "[--optimize_parameters]",
        { "--no_dtam",
          "--no_dtam_tracking",
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
          "--calibration_rotations",
          "--epipole_radius",
          "--nskipped",
          "--nimages",
          "--ncameras",
          "--tracking_mode",
          "--sift_nframes",
          "--fov_min",
          "--fov_max",
          "--features_down_sampling",
          "--dtam_down_sampling",
          "--cost_volume",
          "--regularization",
          "--regularization_filter_sigma",
          "--regularization_filter_poly_degree",
          "--regularization_lambda" });

    try {
        auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

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
                .tracking_mode = tracking_mode_from_string(args.named_value("--tracking_mode", "patches")),
                .regularization = regularization_from_string(args.named_value("--regularization", "dense_geometry")),
                .sift_nframes = safe_stoz(args.named_value("--sift_nframes", "19")),
                .fov_distances = {
                    safe_stof(args.named_value("--fov_min", "0.1")),
                    safe_stof(args.named_value("--fov_max", "100"))},
                .regularization_lambda = safe_stof(args.named_value("--regularization_lambda")),
                .enable_dtam = !args.has_named("--no_dtam"),
                .track_using_dtam = !args.has_named("--no_dtam_tracking"),
                .use_virtual_camera = args.has_named("--use_virtual_camera"),
                .print_residual = args.has_named("--print_residual"),
                .features_down_sampling = safe_stoz(args.named_value("--features_down_sampling", "0")),
                .dtam_down_sampling = safe_stoz(args.named_value("--dtam_down_sampling", "0")),
                .regularization_filter_sigma = safe_stof(args.named_value("--regularization_filter_sigma", "1")),
                .regularization_filter_poly_degree = safe_stoz(args.named_value("--regularization_filter_poly_degree", "0")),
                .optimize_parameters = args.has_named("--optimize_parameters"),
                .calibration_rotations = safe_stoi(args.named_value("--calibration_rotations", "0")),
                .epipole_radius = safe_stof(args.named_value("--epipole_radius", "0")) });
        return 0;
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
}
