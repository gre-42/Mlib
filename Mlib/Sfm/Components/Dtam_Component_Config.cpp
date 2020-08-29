#include "Dtam_Component_Config.hpp"
#include <stdint.h>

using namespace Mlib;
using namespace Mlib::Sfm;

static bool interactive = true;
static float theta_0 = interactive ? 0.2 : 8;

DtamComponentConfig::DtamComponentConfig(bool track_using_dtam)
: DtamComponentConfig(
    0,                              // tracking_start_ncams
    !interactive,                   // rewind_first_keyframe
    interactive ? SIZE_MAX : 5,     // nframes_between_keyframes
    track_using_dtam,               // track_using_dtam
    1,                              // nth_image
    DtamKeyframeConfig(
        interactive,                // incremental_update
        interactive ? 20 : 20,      // nfuture_frames_per_keyframe
        interactive ? 20 :  1,      // npast_frames_per_keyframe
        3 * (interactive ? 1 : 10), // min_channel_increments
        0.8,                        // min_pixel_fraction_for_tracking
        100,                        // ninterleaved_iterations
        Dm::DtamParameters(
            0.5f,                   // min_depth
            5.f,                    // max_depth
            32,                     // ndepths
            100,                    // alpha_G
            1.6,                    // beta_G
            theta_0,                // theta_0 (0.2)
            theta_0 / 0.2 * 1e-4,   // theta_end (1e-4)
            0.0001,                 // beta (0.0001 - 0.001)
            1,                      // lambda (1 for the first keyframe)
            0.2,                    // epsilon (1e-4)
            400)))                  // nsteps
{}

DtamComponentConfig::DtamComponentConfig(
    size_t tracking_start_ncams,
    bool rewind_first_keyframe,
    size_t nframes_between_keyframes,
    bool track_using_dtam,
    size_t nth_image,
    DtamKeyframeConfig keyframe_config)
: tracking_start_ncams_(tracking_start_ncams),
  rewind_first_keyframe_(rewind_first_keyframe),
  nframes_between_keyframes_(nframes_between_keyframes),
  track_using_dtam_(track_using_dtam),
  nth_image_(nth_image),
  keyframe_config_(keyframe_config)
{}
