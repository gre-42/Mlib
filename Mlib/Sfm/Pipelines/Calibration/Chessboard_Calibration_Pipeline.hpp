#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Pipelines/ImagePipeline.hpp>

namespace Mlib::Sfm {

class ImagePipeline;

class ChessboardCalibrationPipeline: public ImagePipeline {
public:
    ChessboardCalibrationPipeline(
        const std::string& cache_dir,
        const ArrayShape& chessboard_shape);
    virtual void process_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame,
        const CameraFrame* camera_frame = nullptr,
        bool is_last_frame = false,
        bool camera_is_initializer = false) override;
    virtual void print_statistics(std::ostream& ostream) override;
    const TransformationMatrix<float, float, 2>& intrinsic_matrix() const;
    const FixedArray<size_t, 2>& sensor_size() const;
    bool is_cached() const;
private:
    Array<FixedArray<float, 2>> p_x_;
    std::list<Array<FixedArray<float, 2>>> p_y_;
    TransformationMatrix<float, float, 2> intrinsic_matrix_;
    FixedArray<size_t, 2> sensor_size_;
    bool is_cached_;
    const ArrayShape chessboard_shape_;
    const std::string cache_dir_;
    const std::string camera_intrinsics_filename_;
    const std::string camera_sensor_size_filename_;
};

}
