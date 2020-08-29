#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Pipelines/ImagePipeline.hpp>

namespace Mlib { namespace Sfm {

class ImagePipeline;

class ChessboardCalibrationPipeline: public ImagePipeline {
public:
    ChessboardCalibrationPipeline(
        const std::string& cache_dir,
        const ArrayShape& chessboard_shape);
    virtual void process_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame,
        const CameraFrame* camera_frame = nullptr) override;
    virtual void print_statistics(std::ostream& ostream) override;
    const Array<float>& intrinsic_matrix() const;
    bool is_cached() const;
private:
    Array<float> p_x_;
    std::list<Array<float>> p_y_;
    Array<float> intrinsic_matrix_;
    bool is_cached_;
    const ArrayShape chessboard_shape_;
    const std::string cache_dir_;
    const std::string camera_intrinsics_filename_;
};

}}
