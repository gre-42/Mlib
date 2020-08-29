#pragma once
#include <Mlib/Sfm/Components/Dtam_Keyframe_Config.hpp>

namespace Mlib { namespace Sfm {

struct DtamComponentConfig {
    explicit DtamComponentConfig(bool track_using_dtam = true);
    DtamComponentConfig(
        size_t tracking_start_ncams,
        bool rewind_first_keyframe,
        size_t nframes_between_keyframes,
        bool track_using_dtam,
        size_t nth_image,
        DtamKeyframeConfig keyframe_config);
    size_t tracking_start_ncams_;
    bool rewind_first_keyframe_;
    size_t nframes_between_keyframes_;
    bool track_using_dtam_;
    size_t nth_image_;
    DtamKeyframeConfig keyframe_config_;
};

}}
