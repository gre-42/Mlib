#include "Chessboard_Calibration_Pipeline.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/StbImage3.hpp>
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
    : intrinsic_matrix_{ uninitialized }
    , sensor_size_{ uninitialized }
    , chessboard_shape_{ chessboard_shape }
    , cache_dir_{ (fs::path{ cache_dir } / "Calibration").string() }
    , camera_intrinsics_filename_{ (fs::path{cache_dir_} / "camera_intrinsic.m").string() }
    , camera_sensor_size_filename_{ (fs::path{cache_dir_} / "camera_sensor_size.m").string() }
{
    if (fs::exists(camera_intrinsics_filename_) &&
        fs::exists(camera_sensor_size_filename_))
    {
        lerr() <<
            "Camera calibration file " << camera_intrinsics_filename_ <<
            " and sensor size file " << camera_sensor_size_filename_ <<
            " exist, loading...";
        Array<float> ki = Array<float>::load_txt_2d(camera_intrinsics_filename_);
        if (!all(ki.shape() == ArrayShape{ 3, 3 })) {
            throw std::runtime_error("Intrinsic matrix has incorrect shape");
        }
        intrinsic_matrix_ = TransformationMatrix<float, float, 2>{ FixedArray<float, 3, 3>{ki} };
        Array<size_t> sz = Array<size_t>::load_txt_2d(camera_sensor_size_filename_);
        if (!all(sz.shape() == ArrayShape{ 2, 1 })) {
            throw std::runtime_error("Sensor size array has incorrect shape");
        }
        sensor_size_ = FixedArray<size_t, 2> { sz.flattened() };
        if (any(sensor_size_ == SIZE_MAX)) {
            throw std::runtime_error("Sensor size array constains invalid values");
        }
        is_cached_ = true;
    } else {
        lerr() <<
            "Camera calibration file " << camera_intrinsics_filename_ <<
            " or sensor size file " << camera_sensor_size_filename_ <<
            " does not exist, computing...";
        sensor_size_ = SIZE_MAX;
        is_cached_ = false;
    }
}

void ChessboardCalibrationPipeline::process_image_frame(
    const std::chrono::milliseconds& time,
    const ImageFrame& image_frame,
    const CameraFrame* camera_frame,
    bool is_last_frame,
    bool camera_is_initializer)
{
    assert(camera_frame == nullptr);
    assert(!camera_is_initializer);
    FixedArray<size_t, 2> sz{image_frame.grayscale.shape(id1), image_frame.grayscale.shape(id0) };
    if (all(sensor_size_ == SIZE_MAX)) {
        sensor_size_ = sz;
    }
    if (any(sensor_size_ != sz)) {
        throw std::runtime_error("Conflicting calibration file shapes");
    }
    Array<FixedArray<float, 2>> p_y;
    std::string p_x_filename = (fs::path{ cache_dir_ } / "features-p_x.m").string();
    std::string p_y_filename = (fs::path{ cache_dir_ } / ("features-" + std::to_string(p_y_.size()) + "-p_y.m")).string();
    if (fs::exists(p_y_filename)) {
        if (p_y_.size() == 0) {
            p_x_ = Array<float>::from_dynamic<2>(Array<float>::load_txt_2d(p_x_filename, ArrayShape{0, 2}));
        }
        p_y = Array<float>::from_dynamic<2>(Array<float>::load_txt_2d(p_y_filename, ArrayShape{0, 2}));
    } else {
        StbImage3 bmp;
        detect_chessboard(
            image_frame.grayscale,
            chessboard_shape_,
            p_x_,
            p_y,
            bmp);
        fs::create_directories(cache_dir_);
        if (p_y_.size() == 0) {
            Array<float>{ p_x_ }.save_txt_2d(p_x_filename);
        }
        Array<float>{ p_y }.save_txt_2d(p_y_filename);
        bmp.save_to_file((fs::path{ cache_dir_ } / ("chessboard" + std::to_string(p_y_.size()) + ".png")).string());
    }
    p_y_.push_back(p_y);
}

void ChessboardCalibrationPipeline::print_statistics(std::ostream& ostream) {
    Array<FixedArray<float, 2>> y{ p_y_ };
    NormalizedProjection np{ y };
    TransformationMatrix<float, float, 2> ki_out = uninitialized;
    Array<FixedArray<float, 3>> p_x_lifted_ = p_x_.applied< FixedArray<float, 3>>([](const auto& p) {return homogenized_3(p); });
    find_projection_matrices(
        p_x_lifted_,    // x
        np.yn,          // y
        nullptr,        // ki_precomputed
        nullptr,        // kep_initial
        &ki_out,        // ki_out
        nullptr,        // ke_out
        nullptr,        // kep_out
        nullptr,        // x_out
        float{ 1e-5 },  // alpha
        float{ 1e-5 },  // beta
        float{ 1e-6 },  // alpha2
        float{ 1e-6 },  // beta2
        -INFINITY,      // min_redux
        100,            // niterations
        5,              // nburnin
        300,            // nmisses
        true,           // print_residual
        true);          // nothrow
    intrinsic_matrix_ = np.denormalized_intrinsic_matrix(ki_out);
    intrinsic_matrix_.affine().to_array().save_txt_2d(camera_intrinsics_filename_);
    sensor_size_.as_column_vector().to_array().save_txt_2d(camera_sensor_size_filename_);
}

const TransformationMatrix<float, float, 2>& ChessboardCalibrationPipeline::intrinsic_matrix() const {
    return intrinsic_matrix_;
}

const FixedArray<size_t, 2>& ChessboardCalibrationPipeline::sensor_size() const {
    return sensor_size_;
}

bool ChessboardCalibrationPipeline::is_cached() const {
    return is_cached_;
}
