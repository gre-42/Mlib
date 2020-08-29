#include "Chessboard_Calibration_Pipeline.hpp"
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Components/Detect_Chessboard.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>
#include <filesystem>

namespace fs = std::filesystem;


using namespace Mlib;
using namespace Mlib::Sfm;

ChessboardCalibrationPipeline::ChessboardCalibrationPipeline(
    const std::string& cache_dir,
    const ArrayShape& chessboard_shape)
    :chessboard_shape_(chessboard_shape),
     cache_dir_(cache_dir + "/Calibration"),
     camera_intrinsics_filename_(cache_dir_ + "/camera_intrinsic.m")
{
    if (fs::exists(camera_intrinsics_filename_)) {
        std::cerr << "Camera calibration file " << camera_intrinsics_filename_ << " exists, loading..." << std::endl;
        intrinsic_matrix_ = Array<float>::load_txt_2d(camera_intrinsics_filename_);
        if (!all(intrinsic_matrix_.shape() == ArrayShape{3, 3})) {
            throw std::runtime_error("Intrinsic matrix has incorrect shape");
        }
        is_cached_ = true;
    } else {
        std::cerr << "Camera calibration file " << camera_intrinsics_filename_ << " does not exist, computing..." << std::endl;
        is_cached_ = false;
    }
}

void ChessboardCalibrationPipeline::process_image_frame(
    const std::chrono::milliseconds& time,
    const ImageFrame& image_frame,
    const CameraFrame* camera_frame)
{
    assert(camera_frame == nullptr);
    PpmImage bmp;
    Array<float> im(image_frame.grayscale);
    Array<float> p_y;
    std::string p_x_filename = cache_dir_ + "/features-p_x.m";
    std::string p_y_filename = cache_dir_ + "/features-" + std::to_string(p_y_.size()) + "-p_y.m";
    if (fs::exists(p_y_filename)) {
        if (p_y_.size() == 0) {
            p_x_ = Array<float>::load_txt_2d(p_x_filename, ArrayShape{0, 4});
        }
        p_y = Array<float>::load_txt_2d(p_y_filename, ArrayShape{0, 3});
    } else {
        detect_chessboard(
            image_frame.grayscale,
            chessboard_shape_,
            p_x_,
            p_y,
            bmp);
        if (p_y_.size() == 0) {
            p_x_.save_txt_2d(p_x_filename);
        }
        p_y.save_txt_2d(p_y_filename);
        bmp.save_to_file(cache_dir_ + "/chessboard" + std::to_string(p_y_.size()) + ".ppm");
    }
    p_y_.push_back(p_y);
}

void ChessboardCalibrationPipeline::print_statistics(std::ostream& ostream) {
    Array<float> y(p_y_);
    NormalizedProjection np(y);
    Array<float> ki_out;
    find_projection_matrices(
        p_x_,         // x
        np.yn,        // y
        nullptr,      // ki_precomputed
        nullptr,      // kep_initial
        &ki_out,      // ki_out
        nullptr,      // ke_out
        nullptr,      // kep_out
        nullptr,      // x_out
        1e-5,         // alpha
        1e-5,         // beta
        1e-6,         // alpha2
        1e-6,         // beta2
        -INFINITY,    // min_redux
        100,          // niterations
        5,            // nburnin
        300,          // nmisses
        true,         // print_residual
        true);        // nothrow
    intrinsic_matrix_ = np.denormalized_intrinsic_matrix(ki_out);
    intrinsic_matrix_.save_txt_2d(camera_intrinsics_filename_);
}

const Array<float>& ChessboardCalibrationPipeline::intrinsic_matrix() const {
    return intrinsic_matrix_;
}

bool ChessboardCalibrationPipeline::is_cached() const {
    return is_cached_;
}
