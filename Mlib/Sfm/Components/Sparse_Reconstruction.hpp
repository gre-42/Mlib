#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
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

namespace Mlib::Sfm {

class SparseReconstruction {
public:
    explicit SparseReconstruction(
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        std::map<size_t, std::chrono::milliseconds>& bad_points,
        std::map<size_t, float>& last_sq_residual,
        const std::string& cache_dir,
        ReconstructionConfig cfg);
    void reconstruct(bool is_last_frame = false, bool camera_initializer_set = false);
    void reconstruct_pass2();
    const Array<FixedArray<float, 3>> reconstructed_points() const;
    const Array<size_t> reconstructed_point_ids() const;
    void set_camera_frame(const std::chrono::milliseconds& time, const CameraFrame& frame);
    CameraFrame& get_camera_frame(const std::chrono::milliseconds& time);
    void print_arrays() const;
private:
    [[nodiscard]] bool compute_reconstruction_pair(
        Array<size_t>& ids,
        Array<FixedArray<float, 2>>& y0,
        Array<FixedArray<float, 2>>& y1,
        std::pair<std::chrono::milliseconds, std::chrono::milliseconds>& times);
    void reconstruct_initial_with_camera();
    void reconstruct_initial_with_svd();
    void reconstruct_initial_for_bundle_adjustment();
    bool is_point_observation_bad(size_t id, const std::chrono::milliseconds& time);
    CameraFrame& camera_frame_append(const std::chrono::milliseconds& time);
    void append_with_projection(const std::chrono::milliseconds& time);
    void append_with_stereo(const std::chrono::milliseconds& time);
    void reconstruct_append();
    void partial_bundle_adjustment(const std::list<std::chrono::milliseconds>& times);
    void global_bundle_adjustment(bool marginalize = true);
    void global_bundle_adjustment_lvm();
    void reject_large_projection_residuals(const GlobalBundle& gb);
    void delete_bad_points();
    void insert_missing_cameras_by_interpolation();
    void insert_missing_cameras_by_append();
    void insert_missing_cameras();
#ifdef REJECT_LARGE_RESIDUALS
    void reject_large_projection_residuals(
        const Array<float>& residual,
        const Array<size_t>& ids,
        const std::list<std::chrono::milliseconds>& times);
#endif
    void draw(const std::string& prefix) const;
    void save_reconstructed(const std::string& prefix) const;
    void save_intrinsic_coefficients(const std::string& prefix) const;
    MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>> reconstructed_points_;
    std::map<size_t, std::shared_ptr<ReconstructedPoint>> frozen_reconstructed_points_;
    TransformationMatrix<float, float, 2> initial_intrinsic_matrix_;
    FixedArray<float, 4> packed_intrinsic_coefficients_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    std::map<std::chrono::milliseconds, CameraFrame> frozen_camera_frames_;
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames_;
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles_;
    std::map<size_t, std::chrono::milliseconds>& bad_points_;
    std::map<size_t, float>& last_sq_residual_;
    std::string cache_dir_;
    ReconstructionConfig cfg_;
    MarginalizationScheduler ms_;
    UUIDGen<XKi, XKe, XP> uuid_gen_;
    std::set<PointObservation> dropped_observations_;
};

}
