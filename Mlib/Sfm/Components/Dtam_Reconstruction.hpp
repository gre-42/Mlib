#pragma once
#include <Mlib/Sfm/Components/Dtam_Component_Config.hpp>
#include <Mlib/Sfm/Components/Dtam_Keyframe.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

namespace Mlib::Sfm {

class DownSampler;

class DtamReconstruction {
public:
    DtamReconstruction(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        DepthMapBundle& depth_map_bundle,
        const DownSampler& down_sampler,
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const std::string& cache_dir,
        const DtamComponentConfig& cfg);
    DtamReconstruction(const DtamReconstruction&) = delete;
    DtamReconstruction& operator = (const DtamReconstruction&) = delete;
    void reconstruct(bool camera_frame_appended_externally, bool camera_computed_with_sift);
    bool can_track_on_its_own() const;
private:
    void insert_keyframe(const std::chrono::milliseconds& keyframe_time, bool camera_computed_with_sift);
    std::map<std::chrono::milliseconds, DtamKeyframe> key_frames_;
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    DepthMapBundle& depth_map_bundle_;
    const DownSampler& down_sampler_;
    TransformationMatrix<float, float, 2> intrinsic_matrix_;
    std::string cache_dir_;
    DtamComponentConfig cfg_;
};

}
