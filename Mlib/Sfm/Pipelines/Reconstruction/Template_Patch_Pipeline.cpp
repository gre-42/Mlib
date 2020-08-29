#include "Template_Patch_Pipeline.hpp"
#include <Mlib/Debug_Prefix.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <iomanip>

using namespace Mlib;
using namespace Mlib::Sfm;

TemplatePatchPipeline::TemplatePatchPipeline(
    const std::string& cache_dir,
    const Array<float>& intrinsic_matrix,
    const TemplatePatchPipelineConfig& cfg)
: intrinsic_matrix_(intrinsic_matrix),
  down_sampler_{intrinsic_matrix, 0},
  flowing_particles_{image_frames_, optical_flows_.optical_flow_frames_, cache_dir + "/TracedParticles", FlowingParticlesConfig()},
  optical_flows_{image_frames_, cache_dir + "/OpticalFlow"},
  sparse_reconstruction_{intrinsic_matrix_, camera_frames_, flowing_particles_.particles_, flowing_particles_.bad_points_, cache_dir + "/SparseReconstruction", ReconstructionConfig()},
  dtam_reconstruction_{image_frames_, camera_frames_, down_sampler_, intrinsic_matrix, cache_dir + "/DtamReconstruction", DtamComponentConfig(cfg.track_using_dtam)},
  cache_dir_(cache_dir),
  cfg_(cfg)
{}

void TemplatePatchPipeline::process_image_frame(
    const std::chrono::milliseconds& time,
    const ImageFrame& image_frame,
    const CameraFrame* camera_frame)
{
    down_sampler_.append_image_frame(time, image_frame);
    image_frames_.insert(std::make_pair(time, image_frame));
    if (camera_frame != nullptr) {
        camera_frames_.insert(std::make_pair(time, *camera_frame));
    }
    if (camera_frame == nullptr && !dtam_reconstruction_.can_track_on_its_own()) {
        if (image_frames_.size() >= 2 && flowing_particles_.requires_optical_flow()) {
            optical_flows_.compute_optical_flow();
        }
        flowing_particles_.advance_flowing_particles();
        sparse_reconstruction_.reconstruct();
    }
    if (cfg_.enable_dtam) {
        // sparse_reconstruction_ may or may have not appended a new camera frame
        dtam_reconstruction_.reconstruct(camera_frames_.find(time) != camera_frames_.end());
    }
}

void TemplatePatchPipeline::save_cameras() const {
    for(const auto& c : camera_frames_) {
        std::stringstream sstr;
        sstr.fill('0');
        sstr << std::setw(6) << c.first.count();
        c.second.projection_matrix_3x4().save_txt_2d(cache_dir_ + "/Cameras/projection-" + sstr.str() + "-3x4.m");
    }
}

void TemplatePatchPipeline::print_statistics(std::ostream& ostream) {
    sparse_reconstruction_.reconstruct_pass2();
    save_cameras();
    ostream << "Statistics:" << std::endl;
}
