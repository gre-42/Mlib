#include "Dtam_Reconstruction.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Mlib;
using namespace Mlib::Sfm;

DtamReconstruction::DtamReconstruction(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    DepthMapBundle& depth_map_bundle,
    const DownSampler& down_sampler,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const std::string& cache_dir,
    const DtamComponentConfig& cfg)
: image_frames_{image_frames},
  camera_frames_{camera_frames},
  depth_map_bundle_{depth_map_bundle},
  down_sampler_{down_sampler},
  intrinsic_matrix_{intrinsic_matrix},
  cache_dir_{cache_dir},
  cfg_{cfg}
{
    fs::create_directories(cache_dir);
}

void DtamReconstruction::insert_keyframe(const std::chrono::milliseconds& keyframe_time, bool camera_computed_with_sift) {
    if (key_frames_.find(keyframe_time) != key_frames_.end()) {
        throw std::runtime_error("Keyframe time conflict");
    }
    lerr() << "nkeyframes " << key_frames_.size();
    key_frames_.insert(std::make_pair(
        keyframe_time,
        DtamKeyframe(
            image_frames_,
            camera_frames_,
            key_frames_,
            depth_map_bundle_,
            down_sampler_,
            intrinsic_matrix_,
            cache_dir_,
            cfg_.keyframe_config_,
            keyframe_time,
            camera_computed_with_sift)));
}

void DtamReconstruction::reconstruct(bool camera_frame_appended_externally, bool camera_computed_with_sift) {
    if (((image_frames_.size() % cfg_.nth_image_) != 0) &&
        (!cfg_.track_using_dtam_ || can_track_on_its_own()))
    {
        return;
    }
    if (!camera_frame_appended_externally && can_track_on_its_own()) {
        DtamKeyframe* kf = DtamKeyframe::currently_tracking_keyframe(key_frames_);
        if (kf == nullptr) {
            throw std::runtime_error("No keyframe finished yet");
        }
        kf->append_camera_frame();
    }
    if (camera_frame_appended_externally && can_track_on_its_own()) {
        const DtamKeyframe* kf = DtamKeyframe::currently_tracking_keyframe(key_frames_);
        if (kf == nullptr) {
            throw std::runtime_error("No keyframe finished yet");
        }
        kf->inspect_externally_appended_camera_frame();
    }
    auto cams_sorted = camera_frames_.sorted();
    if (cams_sorted.size() != 0) {
        lerr() << "DTAM reconstruction [" << cams_sorted.size() << "] = " << cams_sorted.rbegin()->first.count() << " ms";
    }
    if (key_frames_.empty() && !cams_sorted.empty()) {
        if (cams_sorted.size() >= 2) {
            if (cfg_.rewind_first_keyframe_) {
                size_t i = 0;
                for (const auto& c : cams_sorted) {
                    if (i % cfg_.nframes_between_keyframes_ == 0) {
                        lerr() << "Inserting rewinded keyframe at " << c.first.count() << " ms";
                        insert_keyframe(c.first, camera_computed_with_sift && (c.first == cams_sorted.begin()->first));
                    }
                    ++i;
                }
            } else {
                // rbegin counts from behind, while % counts from the first element.
                // times are therefore different to those from modulo calculation.
                // [0, 1, 2, 3, 4], [5, 6, 7, 8, 9], [10, 11, ...]
                lerr() << "Inserting first keyframe at " << cams_sorted.rbegin()->first.count() << " ms";
                insert_keyframe(cams_sorted.rbegin()->first, camera_computed_with_sift);
            }
        }
    } else if (!key_frames_.empty()) {
        assert_true(!camera_computed_with_sift);
        if (cfg_.nframes_between_keyframes_ == SIZE_MAX) {
            if (!key_frames_.rbegin()->second.can_track()) {
                insert_keyframe(cams_sorted.rbegin()->first, false);
            }
        } else {
            while(true) {
                auto last_keyframe_cam = cams_sorted.find(key_frames_.rbegin()->first);
                if (size_t(std::distance(last_keyframe_cam, cams_sorted.end())) > cfg_.nframes_between_keyframes_) {
                    std::advance(last_keyframe_cam, cfg_.nframes_between_keyframes_);
                    lerr() << "Inserting keyframe at " << last_keyframe_cam->first.count() << " ms";
                    insert_keyframe(last_keyframe_cam->first, false);
                } else {
                    break;
                }
            }
        }
    }
    if (cfg_.nframes_between_keyframes_ == SIZE_MAX) {
        if (!key_frames_.empty()) {
            key_frames_.rbegin()->second.reconstruct();
        }
    } else {
        for (auto& k : key_frames_) {
            k.second.reconstruct();
        }
    }
}

bool DtamReconstruction::can_track_on_its_own() const {
    bool res = cfg_.track_using_dtam_ &&
        (camera_frames_.size() > cfg_.tracking_start_ncams_) &&
        (DtamKeyframe::currently_tracking_keyframe(key_frames_) != nullptr);
    lerr() << "can_track_on_its_own = " << res << ", #cams = " << camera_frames_.size();
    return res;
}
