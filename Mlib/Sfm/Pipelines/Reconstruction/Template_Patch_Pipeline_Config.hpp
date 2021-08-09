#pragma once
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib::Sfm {

struct TemplatePatchPipelineConfig {
    bool enable_dtam = true;
    bool track_using_dtam = true;
    bool print_residual = true;
    size_t dtam_down_sampling = 0;
};

}
