#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Sfm/Components/Down_Sampler.hpp>
#include <Mlib/Sfm/Components/Dtam_Component_Config.hpp>
#include <Mlib/Sfm/Components/Dtam_Keyframe.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

namespace Mlib { namespace Sfm {

class DtamReconstruction {
public:
    DtamReconstruction(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const DownSampler& down_sampler,
        const Array<float>& intrinsic_matrix,
        std::string cache_dir,
        const DtamComponentConfig& cfg);
    DtamReconstruction(const DtamReconstruction&) = delete;
    DtamReconstruction& operator = (const DtamReconstruction&) = delete;
    void reconstruct(bool camera_frame_appended_externally);
    bool can_track_on_its_own() const;
private:
    void insert_keyframe(const std::chrono::milliseconds& keyframe_time);
    std::map<std::chrono::milliseconds, DtamKeyframe> key_frames_;
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    const DownSampler& down_sampler_;
    Array<float> intrinsic_matrix_;
    std::string cache_dir_;
    DtamComponentConfig cfg_;
};

}}
