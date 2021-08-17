#pragma once
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib::Sfm {

struct TemplatePatchPipelineConfig {
    bool enable_dtam = true;
    bool track_using_dtam = true;
    bool use_virtual_camera = false;
    bool print_residual = true;
    size_t dtam_down_sampling = 0;
    float regularization_filter_sigma = 1.f;
    size_t regularization_filter_poly_degree = 0;
};

}
