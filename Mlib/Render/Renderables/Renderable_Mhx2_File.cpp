#include "Renderable_Mhx2_File.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Mhx2.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>

using namespace Mlib;

RenderableMhx2File::RenderableMhx2File(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    RenderingResources& rendering_resources)
{
    acvas_ = load_mhx2(filename, cfg);
    rva_ = std::make_shared<RenderableColoredVertexArray>(acvas_, nullptr, rendering_resources);
#ifdef DEBUG
    acvas_->check_consistency();
#endif
}

RenderableMhx2File::~RenderableMhx2File()
{}

void RenderableMhx2File::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> RenderableMhx2File::get_animated_arrays() const {
    return rva_->get_animated_arrays();
}

void RenderableMhx2File::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void RenderableMhx2File::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    return rva_->generate_ray(from, to);
}

AggregateMode RenderableMhx2File::aggregate_mode() const {
    return rva_->aggregate_mode();
}

void RenderableMhx2File::set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float>>& poses) {
    std::vector<OffsetAndQuaternion<float>> ms = vectorize_joint_poses(poses);
    std::vector<OffsetAndQuaternion<float>> mt = acvas_->skeleton->absolutify(ms);
    rva_->set_absolute_joint_poses(mt);
    acvas_->bone_indices.clear();
    acvas_->skeleton = nullptr;
#ifdef DEBUG
    acvas_->check_consistency();
#endif
}

void RenderableMhx2File::downsample(size_t n) {
    rva_->downsample(n);
}

std::vector<OffsetAndQuaternion<float>> RenderableMhx2File::vectorize_joint_poses(
    const std::map<std::string, OffsetAndQuaternion<float>>& poses) const
{
    return acvas_->vectorize_joint_poses(poses);
}

const Bone& RenderableMhx2File::skeleton() const {
    if (acvas_->skeleton == nullptr) {
        throw std::runtime_error("Mhx2 file has no skeleton");
    }
    return *acvas_->skeleton;
}
