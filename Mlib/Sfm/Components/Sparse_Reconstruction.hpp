#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Sfm/Components/Sparse_Reconstruction_Config.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalization_Scheduler.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

#undef REJECT_LARGE_RESIDUALS

namespace Mlib { namespace Sfm {

class SparseReconstruction {
public:
    explicit SparseReconstruction(
        const Array<float>& intrinsic_matrix,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        std::map<size_t, std::chrono::milliseconds>& bad_points,
        const std::string& cache_dir,
        ReconstructionConfig cfg);
    void reconstruct();
    void reconstruct_pass2();
    const Array<float> reconstructed_points() const;
    const Array<size_t> reconstructed_point_ids() const;
    void debug_set_camera_frame(std::chrono::milliseconds time, const CameraFrame& frame);
    void print_arrays() const;
private:
    void reconstruct_initial_with_svd();
    void reconstruct_initial_for_bundle_adjustment();
    bool is_point_observation_bad(size_t id, const std::chrono::milliseconds& time);
    CameraFrame& camera_frame_append(const std::chrono::milliseconds& time);
    void reconstruct_append();
    void partial_bundle_adjustment(const std::list<std::chrono::milliseconds>& times);
    void global_bundle_adjustment();
    void global_bundle_adjustment_lvm();
    void reject_large_projection_residuals(const GlobalBundle& gb);
#ifdef REJECT_LARGE_RESIDUALS
    void reject_large_projection_residuals(
        const Array<float>& residual,
        const Array<size_t>& ids,
        const std::list<std::chrono::milliseconds>& times);
#endif
    void draw(const std::string& prefix) const;
    void save_reconstructed(const std::string& prefix) const;
    MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>> reconstructed_points_;
    std::map<size_t, std::shared_ptr<ReconstructedPoint>> frozen_reconstructed_points_;
    Array<float> intrinsic_matrix_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    std::map<std::chrono::milliseconds, CameraFrame> frozen_camera_frames_;
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles_;
    std::map<size_t, std::chrono::milliseconds>& bad_points_;
    std::string cache_dir_;
    ReconstructionConfig cfg_;
    MarginalizationScheduler ms_;
    UUIDGen<XK, XP> uuid_gen_;
    std::set<std::pair<std::chrono::milliseconds, size_t>> dropped_observations_;
};

}}
