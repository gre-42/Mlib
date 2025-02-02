#include "Dtam_Component_Config.hpp"
#include <Mlib/Sfm/Configuration/Reference_Size.hpp>
#include <cmath>
#include <cstdint>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;

static bool interactive = false;

static Dg::DenseGeometryParameters dgp_320{
    .theta_0__ = 0.1f * 0.2f,
    .theta_end__ = 0.1f * 1e-4f,
    .beta = 0.0001f,
    .lambda__ = 200.f * F_320,
    .tau = 1 / 8.f,
    .nsteps = 400,
    .nsteps_inner = 10};
// static Dg::DenseGeometryParameters dgp_640{
//     .theta_0__ = 0.1f * 0.2f,
//     .theta_end__ = float{ 0.1 * 1e-4 },
//     .beta = 0.000005,
//     .lambda__ = 200.f * F_320,
//     .tau = 1 / 8.f,
//     .nsteps = 3200};

DtamComponentConfig::DtamComponentConfig(
    bool track_using_dtam,
    bool use_virtual_camera,
    bool print_residual,
    Regularization regularization,
    float regularization_filter_sigma,
    size_t regularization_filter_poly_degree,
    float regularization_lambda,
    bool optimize_parameters,
    float epipole_radius)
: DtamComponentConfig(
    0,                              // tracking_start_ncams
    !interactive,                   // rewind_first_keyframe
    interactive ? SIZE_MAX : 5,     // nframes_between_keyframes
    track_using_dtam,               // track_using_dtam
    1,                              // nth_image
    DtamKeyframeConfig(
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
            EdgeImageConfig{
                .alpha = 0.f,
                .beta = 2.f,
                .remove_edge_blobs = false},
            0.005f * 0.2f,          // theta_0 (0.2)
            0.005f * 1e-4f,  // theta_end (1e-4)
            4.02438e-05f,   // beta (0.0001 - 0.001)
            regularization_lambda,  // lambda (1 for the first keyframe)
            0.0504097f,             // epsilon (1e-4)
            500),                   // nsteps
        dgp_320,
        {dgp_320, dgp_320},
        Df::DenseFilteringParameters{
            .nsteps = 400,
            .theta_0__ = 0.2f,
            .theta_end__ = 1e-4f,
            .beta = 0.0001f,
            .lambda = 1.f},
        regularization,
        0.5f,                                            // sigma_illumination_removal
        regularization_filter_sigma,                     // regularization_filter_sigma
        regularization_filter_poly_degree,
        Array<float>{3.f, 1.f, 0.f} / F_320,             // registration_sigmas
        Array<float>{float(INFINITY), float(INFINITY)},  // registration_thresholds
        optimize_parameters,
        epipole_radius))
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
