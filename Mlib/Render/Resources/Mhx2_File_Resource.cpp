#include "Mhx2_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Load_Mhx2.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>

using namespace Mlib;

Mhx2FileResource::Mhx2FileResource(
    const std::string& filename,
    const LoadMeshConfig& cfg)
{
    acvas_ = load_mhx2(filename, cfg);
    rva_ = std::make_shared<ColoredVertexArrayResource>(acvas_);
#ifdef DEBUG
    acvas_->check_consistency();
#endif
}

Mhx2FileResource::~Mhx2FileResource()
{}

void Mhx2FileResource::preload() const {
    rva_->preload();
}

void Mhx2FileResource::instantiate_renderable(const InstantiationOptions& options) const
{
    rva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> Mhx2FileResource::get_animated_arrays() const {
    return rva_->get_animated_arrays();
}

AggregateMode Mhx2FileResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}

void Mhx2FileResource::set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) {
    std::vector<OffsetAndQuaternion<float, float>> ms = vectorize_joint_poses(poses);
    std::vector<OffsetAndQuaternion<float, float>> mt = acvas_->skeleton->rebase_to_initial_absolute_transform(ms);
    rva_->set_absolute_joint_poses(mt);
    acvas_->bone_indices.clear();
    acvas_->skeleton = nullptr;
#ifdef DEBUG
    acvas_->check_consistency();
#endif
}

void Mhx2FileResource::downsample(size_t n) {
    rva_->downsample(n);
}

std::vector<OffsetAndQuaternion<float, float>> Mhx2FileResource::vectorize_joint_poses(
    const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const
{
    return acvas_->vectorize_joint_poses(poses);
}

const Bone& Mhx2FileResource::skeleton() const {
    if (acvas_->skeleton == nullptr) {
        throw std::runtime_error("Mhx2 file has no skeleton");
    }
    return *acvas_->skeleton;
}
