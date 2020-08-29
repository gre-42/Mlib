#include "Dtam_Keyframe_Config.hpp"

using namespace Mlib;
using namespace Mlib::Sfm;

DtamKeyframeConfig::DtamKeyframeConfig(
    bool incremental_update,
    size_t nfuture_frames_per_keyframe,
    size_t npast_frames_per_keyframe,
    size_t min_channel_increments,
    float min_pixel_fraction_for_tracking,
    size_t ninterleaved_iterations,
    Dm::DtamParameters params)
: incremental_update_(incremental_update),
  nfuture_frames_per_keyframe_(nfuture_frames_per_keyframe),
  npast_frames_per_keyframe_(npast_frames_per_keyframe),
  min_channel_increments_(min_channel_increments),
  min_pixel_fraction_for_tracking_(min_pixel_fraction_for_tracking),
  ninterleaved_iterations_(ninterleaved_iterations),
  params_(params)
{}
