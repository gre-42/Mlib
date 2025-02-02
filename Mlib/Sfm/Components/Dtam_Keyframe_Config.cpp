#include "Dtam_Keyframe_Config.hpp"
#include <Mlib/Array/Array_Shape.hpp>
#include <Mlib/Sfm/Configuration/Reference_Size.hpp>
#include <stdexcept>

using namespace Mlib;
using namespace Mlib::Sfm;

DtamKeyframeConfig::DtamKeyframeConfig(
    bool use_virtual_camera,
    bool incremental_update,
    size_t nfuture_frames_per_keyframe,
    size_t npast_frames_per_keyframe,
    size_t min_channel_increments,
    float min_pixel_fraction_for_tracking,
    size_t ninterleaved_iterations,
    bool print_residual,
    const CostVolumeParameters& cost_volume_parameters,
    const Dm::DtamParameters& dm_params,
    const Dg::DenseGeometryParameters& dg_params,
    const std::vector<Dg::DenseGeometryParameters>& dp_params,
    const Df::DenseFilteringParameters& df_params,
    Regularization regularization,
    float sigma_illumination_removal,
    float regularization_filter_sigma,
    size_t regularization_filter_poly_degree,
    const Array<float>& registration_sigmas,
    const Array<float>& registration_thresholds,
    bool optimize_parameters,
    float epipole_radius)
: use_virtual_camera_{use_virtual_camera},
  incremental_update_(incremental_update),
  nfuture_frames_per_keyframe__(nfuture_frames_per_keyframe),
  npast_frames_per_keyframe__(npast_frames_per_keyframe),
  min_channel_increments__(min_channel_increments),
  min_pixel_fraction_for_tracking_(min_pixel_fraction_for_tracking),
  ninterleaved_iterations_(ninterleaved_iterations),
  print_residual_{print_residual},
  cost_volume_parameters_{cost_volume_parameters},
  dm_params_(dm_params),
  dg_params_{dg_params},
  dp_params_{dp_params},
  df_params_(df_params),
  regularization_{regularization},
  sigma_illumination_removal_{sigma_illumination_removal},
  regularization_filter_sigma_{regularization_filter_sigma},
  regularization_filter_poly_degree_{regularization_filter_poly_degree},
  registration_sigmas__(registration_sigmas),
  registration_thresholds_(registration_thresholds),
  optimize_parameters_{optimize_parameters},
  epipole_radius_{epipole_radius}
{}

Array<float> DtamKeyframeConfig::registration_sigmas_corrected(const ArrayShape& shape) const {
    return registration_sigmas__ * (float)std::max(image_size(shape), S_320);
}
