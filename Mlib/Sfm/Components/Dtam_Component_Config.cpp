#include "Dtam_Component_Config.hpp"
#include <stdint.h>

using namespace Mlib;
using namespace Mlib::Sfm;

static bool interactive = false;

DtamComponentConfig::DtamComponentConfig(
    bool track_using_dtam,
    bool print_residual)
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
        3 * size_t{ interactive ? 1u : 10u }, // min_channel_increments
        0.8f,                       // min_pixel_fraction_for_tracking
        100,                        // ninterleaved_iterations
        print_residual,             // print_residual
        Dm::DtamParameters(
            5.f,                    // min_depth
            9.f,                    // max_depth
            32,                     // ndepths
            100.f,                  // alpha_G
            1.6f,                   // beta_G
            0.2,                    // theta_0 (0.2)
            float{ 1e-4 },          // theta_end (1e-4)
            0.0001f,                // beta (0.0001 - 0.001)
            1.f,                    // lambda (1 for the first keyframe)
            float{ 1e-4 },          // epsilon (1e-4)
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
