#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Sfm/Components/Down_Sampler.hpp>
#include <Mlib/Sfm/Components/Dtam_Keyframe_Config.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping.hpp>
#include <Mlib/Sfm/Disparity/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>
#include <set>

namespace Mlib { namespace Sfm {

class DtamKeyframe {
public:
    DtamKeyframe(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames,
        const DownSampler& down_sampler,
        const Array<float>& intrinsic_matrix,
        std::string cache_dir,
        const DtamKeyframeConfig& cfg,
        const std::chrono::milliseconds& key_frame_time);
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
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames__;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames_;
    const DownSampler& down_sampler_;
    std::set<std::chrono::milliseconds> times_integrated_;
    Array<float> intrinsic_matrix__;
    std::string cache_dir_;
    std::chrono::milliseconds first_integrated_time_;
    std::chrono::milliseconds last_integrated_time_;
    const std::chrono::milliseconds key_frame_time_;
    std::unique_ptr<InverseDepthCostVolume> vol_;
    std::unique_ptr<Dm::DenseMapping> dm_;
    bool can_track_;
    size_t opt_id_;
    DtamKeyframeConfig cfg_;
    Array<float> ai_;
    Array<float> depth_;
    Array<float> masked_depth_;
    Array<float> dsi_;
};

}}
