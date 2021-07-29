#pragma once
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dense_Filtering_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <cstddef>

namespace Mlib { namespace Sfm {

enum class Regularization {
    DTAM,
    FILTERING
};

struct DtamKeyframeConfig {
    DtamKeyframeConfig(
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
        float sigma_illumination_removal);
    bool rewind_first_keyframe_;
    bool incremental_update_;
    size_t nfuture_frames_per_keyframe_;
    size_t npast_frames_per_keyframe_;
    size_t min_channel_increments_;
    float min_pixel_fraction_for_tracking_;
    size_t ninterleaved_iterations_;
    bool print_residual_;
    CostVolumeParameters cost_volume_parameters_;
    Dm::DtamParameters dm_params_;
    DenseFilteringParameters df_params_;
    Regularization regularization_;
    float sigma_illumination_removal_;
};

}}
