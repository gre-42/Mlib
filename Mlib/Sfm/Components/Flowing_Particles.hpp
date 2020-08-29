#pragma once
#include <Mlib/Sfm/Components/Flowing_Particles_Config.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <set>

#ifdef WITH_OPENCV
#include <opencv2/features2d.hpp>
#endif

namespace Mlib {

class Bgr565Bitmap;

namespace Sfm {

class FlowingParticles {

public:
    FlowingParticles(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        const std::map<std::chrono::milliseconds, ImageFrame>& optical_flow_frames,
        const std::string& cache_dir,
        const FlowingParticlesConfig& cfg_);
    void advance_flowing_particles();
    void draw(Bgr565Bitmap& bmp);
    bool requires_optical_flow() const;
    std::map<std::chrono::milliseconds, FeaturePointFrame> particles_;
    std::map<size_t, std::chrono::milliseconds> bad_points_;
private:
    void advance_existing_particles(
        FeaturePointFrame& new_frame,
        const Array<float>& flow_field,
        const Array<bool>& flow_mask);
    void generate_new_particles(FeaturePointFrame& new_frame);
    size_t streamline_search_length();
    bool try_generate_feature_point_sequence(
        FeaturePointFrame& frame,
        const Array<float>& pos);
    bool try_insert_and_append_feature_point(
        FeaturePointFrame& frame,
        const std::pair<size_t, std::shared_ptr<FeaturePointSequence>>& seq,
        const Array<float>& pos);
    std::shared_ptr<FeaturePoint> generate_feature_point(const Array<float>& pos) const;
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames_;
    const std::map<std::chrono::milliseconds, ImageFrame>& optical_flow_frames_;
#if 0
    cv::Ptr<cv::ORB> orb_;
    cv::BFMatcher bfm_;
#endif
    ArrayShape shape_;
    std::string cache_dir_;
    FlowingParticlesConfig cfg_;
    std::atomic_size_t sequence_index = 0;
};

}}
