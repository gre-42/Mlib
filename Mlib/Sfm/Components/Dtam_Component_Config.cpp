#include "Dtam_Component_Config.hpp"
#include <Mlib/Sfm/Configuration/Reference_Size.hpp>
#include <cmath>
#include <stdint.h>

using namespace Mlib;
using namespace Mlib::Sfm;

static bool interactive = false;

static Dg::DenseGeometryParameters dgp{
    .theta_0__ = 0.1f * 0.2f,
    .theta_end__ = float{ 0.1 * 1e-4 },
    .beta = 0.0001,
    .lambda = 200.f,
    .tau = 1 / 8.f,
    .nsteps = 400};

DtamComponentConfig::DtamComponentConfig(
    bool track_using_dtam,
    bool use_virtual_camera,
    bool print_residual,
    Regularization regularization,
    float regularization_filter_sigma,
    size_t regularization_filter_poly_degree,
    bool optimize_parameters)
: DtamComponentConfig(
    0,                              // tracking_start_ncams
    !interactive,                   // rewind_first_keyframe
    interactive ? SIZE_MAX : 5,     // nframes_between_keyframes
    track_using_dtam,               // track_using_dtam
    1,                              // nth_image
    DtamKeyframeConfig(
        false,                      // rewind_first_keyframe
        use_virtual_camera,         // use_virtual_camera
        interactive,                // incremental_update
        interactive ? 20 : 20,      // nfuture_frames_per_keyframe
        interactive ? 20 :  1,      // npast_frames_per_keyframe
        3 * size_t{ interactive ? 1u : 10u }, // min_channel_increments
        0.8f,                       // min_pixel_fraction_for_tracking
        100,                        // ninterleaved_iterations
        print_residual,             // print_residual
        CostVolumeParameters{
            .min_depth = 3.5f,
            .max_depth = 12.f,
            .ndepths = 32
        },
        Dm::DtamParameters(
            100.f,                  // alpha_G
            1.6f,                   // beta_G
            0.2,                    // theta_0 (0.2)
            float{ 1e-4 },          // theta_end (1e-4)
            0.0001f,                // beta (0.0001 - 0.001)
            1.f,                    // lambda (1 for the first keyframe)
            float{ 1e-4 },          // epsilon (1e-4)
            400),                   // nsteps
        dgp,
        {dgp, dgp},
        Df::DenseFilteringParameters{
            .nsteps = 400,
            .theta_0__ = 0.2,
            .theta_end__ = float{ 1e-4 },
            .beta = 0.0001,
            .lambda = 1.f},
        regularization,
        0.5f,                                            // sigma_illumination_removal
        regularization_filter_sigma,                     // regularization_filter_sigma
        regularization_filter_poly_degree,
        Array<float>{3.f, 1.f, 0.f} / F_320,             // registration_sigmas
        Array<float>{float(INFINITY), float(INFINITY)},  // registration_thresholds
        optimize_parameters))
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
