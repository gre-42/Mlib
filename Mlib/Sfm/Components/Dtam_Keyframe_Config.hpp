#pragma once
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <cstddef>

namespace Mlib { namespace Sfm {

struct DtamKeyframeConfig {
    DtamKeyframeConfig(
        bool incremental_update,
        size_t nfuture_frames_per_keyframe,
        size_t npast_frames_per_keyframe,
        size_t min_channel_increments,
        float min_pixel_fraction_for_tracking,
        size_t ninterleaved_iterations,
        Dm::DtamParameters params);
    bool rewind_first_keyframe_;
    bool incremental_update_;
    size_t nfuture_frames_per_keyframe_;
    size_t npast_frames_per_keyframe_;
    size_t min_channel_increments_;
    float min_pixel_fraction_for_tracking_;
    size_t ninterleaved_iterations_;
    Dm::DtamParameters params_;
};

}}
