#pragma once
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib::Sfm {

enum class TrackingMode;
enum class Regularization;

struct TemplatePatchPipelineConfig {
    TrackingMode tracking_mode;
    Regularization regularization;
    size_t sift_nframes;
    FixedArray<float, 2> fov_distances;
    float regularization_lambda;
    bool enable_dtam = true;
    bool track_using_dtam = true;
    bool use_virtual_camera = false;
    bool print_residual = true;
    size_t features_down_sampling = 0;
    size_t dtam_down_sampling = 0;
    float regularization_filter_sigma = 1.f;
    size_t regularization_filter_poly_degree = 0;
    bool optimize_parameters = false;
    int calibration_rotations = 0;
    float epipole_radius = 0.f;
};

}
