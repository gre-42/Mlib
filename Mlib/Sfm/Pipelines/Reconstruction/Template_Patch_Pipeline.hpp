#pragma once
#include <Mlib/Sfm/Components/Down_Sampler.hpp>
#include <Mlib/Sfm/Components/Dtam_Reconstruction.hpp>
#include <Mlib/Sfm/Components/Flowing_Particles.hpp>
#include <Mlib/Sfm/Components/Optical_Flows.hpp>
#include <Mlib/Sfm/Components/Sparse_Reconstruction.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Pipelines/ImagePipeline.hpp>
#include <Mlib/Sfm/Pipelines/Reconstruction/Template_Patch_Pipeline_Config.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>
#include <ostream>

namespace Mlib { namespace Sfm {

class TemplatePatchPipeline: public ImagePipeline {
public:
    TemplatePatchPipeline(
        const std::string& cache_dir,
        const Array<float>& intrinsic_matrix,
        const TemplatePatchPipelineConfig& cfg);
    virtual void process_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame,
        const CameraFrame* camera_frame = nullptr) override;
    virtual void print_statistics(std::ostream& ostream) override;
private:
    void save_cameras() const;

    Array<float> intrinsic_matrix_;
    DownSampler down_sampler_;
    FlowingParticles flowing_particles_;
    OpticalFlows optical_flows_;
    SparseReconstruction sparse_reconstruction_;
    DtamReconstruction dtam_reconstruction_;
    std::map<std::chrono::milliseconds, ImageFrame> image_frames_;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> camera_frames_;
    std::string cache_dir_;
    TemplatePatchPipelineConfig cfg_;
};

}}
