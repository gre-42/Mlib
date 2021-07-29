#include "Dtam_Keyframe_Config.hpp"
#include <stdexcept>

using namespace Mlib;
using namespace Mlib::Sfm;

DtamKeyframeConfig::DtamKeyframeConfig(
    bool incremental_update,
    size_t nfuture_frames_per_keyframe,
    size_t npast_frames_per_keyframe,
    size_t min_channel_increments,
    float min_pixel_fraction_for_tracking,
    size_t ninterleaved_iterations,
    bool print_residual,
    const CostVolumeParameters& cost_volume_parameters,
    const Dm::DtamParameters& dm_params,
    const DenseFilteringParameters& df_params,
    Regularization regularization,
    float sigma_illumination_removal)
: incremental_update_(incremental_update),
  nfuture_frames_per_keyframe_(nfuture_frames_per_keyframe),
  npast_frames_per_keyframe_(npast_frames_per_keyframe),
  min_channel_increments_(min_channel_increments),
  min_pixel_fraction_for_tracking_(min_pixel_fraction_for_tracking),
  ninterleaved_iterations_(ninterleaved_iterations),
  print_residual_{print_residual},
  cost_volume_parameters_{cost_volume_parameters},
  dm_params_(dm_params),
  df_params_(df_params),
  regularization_{regularization},
  sigma_illumination_removal_{sigma_illumination_removal}
{}
