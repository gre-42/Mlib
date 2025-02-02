#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Components/Down_Sampler.hpp>
#include <Mlib/Sfm/Components/Dtam_Keyframe_Config.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>
#include <set>

namespace Mlib::Sfm {

class InverseDepthCostVolume;
class DenseDepthEstimation;
class DepthMapBundle;
class CostVolumeAccumulator;
class CostVolume;

class DtamKeyframe {
public:
    DtamKeyframe(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames,
        DepthMapBundle& depth_map_bundle,
        const DownSampler& down_sampler,
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const std::string& cache_dir,
        const DtamKeyframeConfig& cfg,
        const std::chrono::milliseconds& key_frame_time,
        bool camera_computed_with_sift);
    DtamKeyframe(DtamKeyframe&&);
    DtamKeyframe(const DtamKeyframe&) = delete;
    DtamKeyframe& operator = (const DtamKeyframe&) = delete;
    ~DtamKeyframe();
    void reconstruct();
    void append_camera_frame();
    void inspect_externally_appended_camera_frame() const;
    bool is_full() const;
    bool can_track() const;
    static const DtamKeyframe* currently_tracking_keyframe(const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames);
    static DtamKeyframe* currently_tracking_keyframe(std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames);
private:
    void update_cost_volume(bool& cost_volume_changed);
    void optimize0(bool cost_volume_changed);
    void optimize1();
    bool past_is_full() const;
    bool future_is_full() const;
    void draw_reconstruction(const std::string& suffix) const;
    size_t nfuture_frames_per_keyframe() const;
    size_t npast_frames_per_keyframe() const;
    size_t min_channel_increments() const;
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames__;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames_;
    DepthMapBundle& depth_map_bundle_;
    const DownSampler& down_sampler_;
    std::set<std::chrono::milliseconds> times_integrated_;
    TransformationMatrix<float, float, 2> intrinsic_matrix__;
    std::string cache_dir_;
    std::chrono::milliseconds first_integrated_time_;
    std::chrono::milliseconds last_integrated_time_;
    const std::chrono::milliseconds key_frame_time_;
    std::unique_ptr<CostVolumeAccumulator> vol_acc_;
    std::unique_ptr<CostVolume> vol_;
    std::unique_ptr<DenseDepthEstimation> dm_;
    bool can_track_;
    size_t opt_id_;
    DtamKeyframeConfig cfg_;
    bool camera_computed_with_sift_;
    Array<float> ai_;
    Array<float> depth_;
    // Array<float> masked_depth_;
    Array<float> dsi_;
};

}
