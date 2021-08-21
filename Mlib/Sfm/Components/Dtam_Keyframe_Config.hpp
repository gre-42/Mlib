#pragma once
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dense_Filtering_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dense_Geometry_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <cstddef>

namespace Mlib::Sfm {

enum class Regularization {
    DTAM,
    DENSE_GEOMETRY,
    FILTERING
};

struct DtamKeyframeConfig {
    DtamKeyframeConfig(
        bool rewind_first_keyframe,
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
        const Df::DenseFilteringParameters& df_params,
        Regularization regularization,
        float sigma_illumination_removal,
        float regularization_filter_sigma,
        size_t regularization_filter_poly_degree);
    bool rewind_first_keyframe_;
    bool use_virtual_camera_;
    bool incremental_update_;
    size_t nfuture_frames_per_keyframe_;
    size_t npast_frames_per_keyframe_;
    size_t min_channel_increments_;
    float min_pixel_fraction_for_tracking_;
    size_t ninterleaved_iterations_;
    bool print_residual_;
    CostVolumeParameters cost_volume_parameters_;
    Dm::DtamParameters dm_params_;
    Dg::DenseGeometryParameters dg_params_;
    Df::DenseFilteringParameters df_params_;
    Regularization regularization_;
    float sigma_illumination_removal_;
    float regularization_filter_sigma_;
    size_t regularization_filter_poly_degree_;
};

}
