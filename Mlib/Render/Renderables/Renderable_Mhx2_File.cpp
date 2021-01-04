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
    rva_ = std::make_shared<RenderableColoredVertexArray>(acvas_->cvas, nullptr, rendering_resources);
}

RenderableMhx2File::~RenderableMhx2File()
{}

void RenderableMhx2File::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableMhx2File::get_triangle_meshes() const {
    return rva_->get_triangle_meshes();
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

void RenderableMhx2File::set_relative_joint_poses(const std::map<std::string, FixedArray<float, 4, 4>>& poses) {
    std::map<std::string, std::string> n2n{
        {"hip", "pelvis"},
        {"abdomen", "spine_01"},
        // spine_02
        {"chest", "spine_03"},
        {"neck", "neck_01"},
        {"head", "head"},
        {"lCollar", "clavicle_l"},
        {"lShldr", "upperarm_l"},
        {"lForeArm", "lowerarm_l"},
        {"lHand", "hand_l"},
        {"rCollar", "clavicle_r"},
        {"rShldr", "upperarm_r"},
        {"rForeArm", "lowerarm_l"},
        {"rHand", "hand_l"},
        {"lButtock", ""},
        {"lThigh", "thigh_l"},
        {"lShin", "calf_l"},
        {"lFoot", "foot_l"},
        {"rButtock", ""},
        {"rThigh", "thigh_r"},
        {"rShin", "calf_r"},
        {"rFoot", "foot_r"},
        {"lIndex1", ""},
        {"lIndex2", ""},
        {"lMid1", ""},
        {"lMid2", ""},
        {"lRing1", ""},
        {"lRing2", ""},
        {"lPinky1", ""},
        {"lPinky2", ""},
        {"rIndex1", ""},
        {"rIndex2", ""},
        {"rMid1", ""},
        {"rMid2", ""},
        {"rRing1", ""},
        {"rRing2", ""},
        {"rPinky1", ""},
        {"rPinky2", ""},
        {"lThumb1", ""},
        {"lThumb2", ""},
        {"rThumb1", ""},
        {"rThumb2", ""},
        {"leftEye", ""},
        {"rightEye", ""}};
    std::vector<FixedArray<float, 4, 4>> ms(acvas_->bone_indices.size());
    for (auto& m : ms) {
        m = fixed_identity_array<float, 4>();
    }
    for (const auto& p : poses) {
        auto nit = n2n.find(p.first);
        if (nit == n2n.end()) {
            throw std::runtime_error("Could not find translation for bone " + p.first);
        }
        if (!nit->second.empty()) {
            auto it = acvas_->bone_indices.find(nit->second);
            if (it == acvas_->bone_indices.end()) {
                throw std::runtime_error("set_relative_joint_poses: Could not find bone with name " + nit->second);
            }
            ms.at(it->second) = p.second;
        }
    }
    std::vector<FixedArray<float, 4, 4>> mt = acvas_->skeleton->absolutify(ms);
    rva_->set_joint_poses(mt);
}
