#include "Gltf_File_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <iostream>
#include <tinygltf/tiny_gltf.hpp>

namespace Mlib {
inline std::ostream& operator << (std::ostream& ostr, const std::vector<double>& vec) {
    for (const auto& v : vec) {
        ostr << v << " ";
    }
    return ostr;
}
}

using namespace Mlib;
namespace tg = tinygltf;

GltfFileResource::GltfFileResource(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    const SceneNodeResources& scene_node_resources)
: hri_{ scene_node_resources }
{
    tg::Model model;
    tg::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret;
    if (filename.ends_with(".gltf")) {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    } else if (filename.ends_with(".glb")) {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    } else {
        throw std::runtime_error("Filename does not end with \".tglf\" or \".glb\"");
    }
    if (!warn.empty()) {
        throw std::runtime_error("GLTF warning while loading file \"" + filename + "\": " + warn);
    }
    if (!err.empty()) {
        throw std::runtime_error("GLTF error while loading file \"" + filename + "\": " + err);
    }
    if (!ret) {
        throw std::runtime_error("Failed to parse GLTF: \"" + filename + '"');
    }
    if (model.scenes.size() != 1) {
        throw std::runtime_error("Did not find exactly one scene in GLTF file \"" + filename + '"');
    }
    if (model.defaultScene != 0) {
        throw std::runtime_error("Default scene is not zero in GLTF file \"" + filename + '"');
    }
    const tg::Scene &scene = model.scenes[model.defaultScene];
    std::function<void(const std::vector<int>&)> traverse_nodes =
        [&model, &traverse_nodes](const std::vector<int>& nodes)
    {
        for (int i : nodes) {
            if ((i < 0) || ((size_t)i > model.nodes.size())) {
                throw std::runtime_error("Node index out of bounds");
            }
            const auto& node = model.nodes[i];
            
            std::cerr << "Found node " << node.name << std::endl;
            std::cerr << "Rotation:    " << node.rotation << std::endl;
            std::cerr << "Scale:       " << node.scale << std::endl;
            std::cerr << "Translation: " << node.translation << std::endl;
            std::cerr << "Matrix:      " << node.matrix << std::endl;
            traverse_nodes(node.children);
        }
    };
    traverse_nodes(scene.nodes);
    // hri_.acvas->scvas = load_obj(filename, cfg);
    rva_ = std::make_shared<ColoredVertexArrayResource>(hri_.acvas);
}

GltfFileResource::~GltfFileResource()
{}

void GltfFileResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const
{
    hri_.instantiate_renderable(
        name,
        scene_node,
        FixedArray<float, 3>{ 0.f, 0.f, 0.f },
        1.f,
        renderable_resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> GltfFileResource::get_animated_arrays() const {
    return hri_.get_animated_arrays(1.f);
}

void GltfFileResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void GltfFileResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    rva_->generate_ray(from, to);
}

std::shared_ptr<SceneNodeResource> GltfFileResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return rva_->generate_grind_lines(edge_angle, averaged_normal_angle, filter);
}

std::shared_ptr<SceneNodeResource> GltfFileResource::generate_contour_edges() const {
    return rva_->generate_contour_edges();
}

void GltfFileResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    return rva_->modify_physics_material_tags(add, remove, filter);
}

void GltfFileResource::generate_instances() {
    hri_.generate_instances();
}

AggregateMode GltfFileResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}

void GltfFileResource::print(std::ostream& ostr) const {
    rva_->print(ostr);
}

void GltfFileResource::downsample(size_t n) {
    rva_->downsample(n);
}

void GltfFileResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    rva_->import_bone_weights(other_acvas, max_distance);
}
