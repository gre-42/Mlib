#pragma once
#include <Mlib/Sfm/Marginalization/Marginalizing_Bias.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Global_Bundle.hpp>

namespace Mlib { namespace Sfm {

class MarginalizationIds;

enum class MarginalizationTarget {
    POINTS,
    CAMERAS
};

class GlobalMarginalizationConfig {
public:
    bool bias_enabled;
    size_t nbundle_cameras;
    MarginalizationTarget marginalization_target;
};

class MarginalizationScheduler {
public:
    MarginalizationScheduler(
        const GlobalMarginalizationConfig& cfg,
        const GlobalBundleConfig& bundle_cfg,
        const std::string& cache_dir,
        std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
        const Array<float>& intrinsic_matrix,
        bool skip_missing_cameras,
        UUIDGen<XK, XP>& uuid_gen,
        std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations,
        const std::map<size_t, std::chrono::milliseconds>& bad_points);

    std::unique_ptr<GlobalBundle> marginalize();
    MarginalizingBias bsolver_;

private:
    std::chrono::milliseconds find_time_to_be_marginalized_npoints() const;
    std::chrono::milliseconds find_time_to_be_marginalized_distance() const;

    std::vector<size_t> find_points_to_be_marginalized(
        const std::chrono::milliseconds& time_tbm,
        MarginalizationIds& mids);
    void find_cameras_to_be_linearized(
        const std::vector<size_t>& point_ids,
        MarginalizationIds& mids);
    void find_points_to_be_linearized(
        const std::chrono::milliseconds& time,
        MarginalizationIds& mids);
    void abandon_points();

    const GlobalMarginalizationConfig cfg_;
    const GlobalBundleConfig bundle_cfg_;
    const std::string cache_dir_;
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles_;
    MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points_;
    std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames_;
    const Array<float>& intrinsic_matrix_;
    bool skip_missing_cameras_;
    UUIDGen<XK, XP>& uuid_gen_;
    std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations_;
    const std::map<size_t, std::chrono::milliseconds>& bad_points_;

};

}}
