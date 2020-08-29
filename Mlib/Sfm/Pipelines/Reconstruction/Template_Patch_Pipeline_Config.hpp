#pragma once
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib { namespace Sfm {

struct TemplatePatchPipelineConfig {
    bool enable_dtam = true;
    bool track_using_dtam = true;
};

}}
