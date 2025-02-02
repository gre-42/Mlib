#include "Template_Patch_Pipeline.hpp"
#include <Mlib/Debug_Prefix.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Sfm/Configuration/Tracking_Mode.hpp>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;

TemplatePatchPipeline::TemplatePatchPipeline(
    const std::string& cache_dir,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TemplatePatchPipelineConfig& cfg)
: intrinsic_matrix_(intrinsic_matrix),
  features_down_sampler_{intrinsic_matrix, cfg.features_down_sampling},
  dtam_down_sampler_{intrinsic_matrix, cfg.dtam_down_sampling},
  cache_dir_(cache_dir),
  cfg_(cfg),
  optical_flows_{features_down_sampler_.ds_image_frames_, (fs::path{cache_dir} / "OpticalFlow").string()},
  flowing_particles_{
      features_down_sampler_.ds_image_frames_,
      optical_flows_.optical_flow_frames_,
      (fs::path{cache_dir} / "TracedParticles").string(),
      FlowingParticlesConfig{
          .tracking_mode = cfg.tracking_mode
      }},
  sparse_reconstruction_{
      features_down_sampler_.ds_intrinsic_matrix_,
      camera_frames_,
      features_down_sampler_.ds_image_frames_,
      flowing_particles_.particles_,
      flowing_particles_.bad_points_,
      flowing_particles_.last_sq_residual_,
      (fs::path{cache_dir} / "SparseReconstruction").string(),
      ReconstructionConfig{
          .sift_nframes = cfg.sift_nframes,
          .print_residual = cfg.print_residual,
          .fov_distances = cfg.fov_distances} },
  depth_map_bundle_(),
  dtam_reconstruction_{
      image_frames_,
      camera_frames_,
      depth_map_bundle_,
      dtam_down_sampler_,
      intrinsic_matrix_,
      (fs::path{cache_dir} / "DtamReconstruction").string(),
      DtamComponentConfig(
          cfg.track_using_dtam,
          cfg.use_virtual_camera,
          cfg.print_residual,
          cfg.regularization,
          cfg.regularization_filter_sigma,
          cfg.regularization_filter_poly_degree,
          cfg.regularization_lambda,
          cfg.optimize_parameters,
          cfg.epipole_radius) }
{}

void TemplatePatchPipeline::process_image_frame(
    const std::chrono::milliseconds& time,
    const ImageFrame& image_frame,
    const CameraFrame* camera_frame,
    bool is_last_frame,
    bool camera_is_initializer)
{
    features_down_sampler_.append_image_frame(time, image_frame);
    dtam_down_sampler_.append_image_frame(time, image_frame);
    image_frames_.insert(std::make_pair(time, image_frame));
    if (camera_frame != nullptr) {
        camera_frames_.insert(std::make_pair(time, *camera_frame));
    }
    if (((camera_frame == nullptr) || camera_is_initializer) && !dtam_reconstruction_.can_track_on_its_own()) {
        if (image_frames_.size() >= 2 && flowing_particles_.requires_optical_flow()) {
            optical_flows_.compute_optical_flow();
        }
        flowing_particles_.advance_flowing_particles();
        sparse_reconstruction_.reconstruct(
            is_last_frame,
            (camera_frame != nullptr) && camera_is_initializer);
    }
    if (cfg_.enable_dtam) {
        // sparse_reconstruction_ may or may not have appended a new camera frame
        dtam_reconstruction_.reconstruct(
            camera_frames_.find(time) != camera_frames_.end(),
            (flowing_particles_.particles_.find(time) != flowing_particles_.particles_.end()) &&
            (flowing_particles_.particles_.at(time).tracking_mode & TrackingMode::SIFT));
    }
}

void TemplatePatchPipeline::save_cameras() const {
    fs::create_directories(fs::path{ cache_dir_ } / "Cameras");
    for (const auto& c : camera_frames_) {
        std::stringstream sstr;
        sstr.fill('0');
        sstr << std::setw(6) << c.first.count();
        c.second.projection_matrix_3x4().semi_affine().to_array().save_txt_2d((fs::path{ cache_dir_ } / "Cameras" / ("projection-" + sstr.str() + "-3x4.m")).string());
    }
}

void TemplatePatchPipeline::print_statistics(std::ostream& ostream) {
    sparse_reconstruction_.reconstruct_pass2();
    save_cameras();
    ostream << "Statistics:" << std::endl;
}
