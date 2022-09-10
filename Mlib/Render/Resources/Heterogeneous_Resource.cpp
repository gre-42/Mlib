#include "Heterogeneous_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <mutex>

using namespace Mlib;

HeterogeneousResource::HeterogeneousResource(
    const SceneNodeResources& scene_node_resources)
: bri{ std::make_unique<BatchResourceInstantiator>() },
  acvas{ std::make_shared<AnimatedColoredVertexArrays>() },
  scene_node_resources_{ scene_node_resources }
{}

HeterogeneousResource::~HeterogeneousResource()
{}

// SceneNodeResource

void HeterogeneousResource::preload() const {
    bri->preload(scene_node_resources_);
    auto preload_textures = [](const auto& cvas) {
        for (const auto& cva : cvas) {
            for (const auto& tex : cva->material.textures) {
                RenderingContextStack::primary_rendering_resources()->preload(tex.texture_descriptor);
            }
        }
    };
    preload_textures(acvas->scvas);
    preload_textures(acvas->dcvas);
}

void HeterogeneousResource::instantiate_renderable(const InstantiationOptions& options) const
{
    instantiate_renderable(
        options,
        FixedArray<float, 3>{ 0.f, 0.f, 0.f },
        1.f);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeterogeneousResource::get_animated_arrays() const {
    return get_animated_arrays(1.f);
}

void HeterogeneousResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    ColoredVertexArrayResource(acvas).generate_triangle_rays(npoints, lengths, delete_triangles);
}

void HeterogeneousResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    ColoredVertexArrayResource(acvas).generate_ray(from, to);
}

std::shared_ptr<SceneNodeResource> HeterogeneousResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return ColoredVertexArrayResource(acvas).generate_grind_lines(edge_angle, averaged_normal_angle, filter);
}

std::shared_ptr<SceneNodeResource> HeterogeneousResource::generate_contour_edges() const {
    return ColoredVertexArrayResource(acvas).generate_contour_edges();
}

void HeterogeneousResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    return ColoredVertexArrayResource(acvas).modify_physics_material_tags(add, remove, filter);
}

AggregateMode HeterogeneousResource::aggregate_mode() const {
    return ColoredVertexArrayResource(acvas).aggregate_mode();
}

void HeterogeneousResource::print(std::ostream& ostr) const {
    ColoredVertexArrayResource(acvas).print(ostr);
}

void HeterogeneousResource::downsample(size_t n) {
    ColoredVertexArrayResource(acvas).downsample(n);
}

void HeterogeneousResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    ColoredVertexArrayResource(acvas).import_bone_weights(other_acvas, max_distance);
}

// Custom

void HeterogeneousResource::instantiate_renderable(
    const InstantiationOptions& options,
    const FixedArray<float, 3>& rotation,
    float scale) const
{
    bri->instantiate_renderables(
        scene_node_resources_,
        options,
        rotation,
        scale);

    do {
        {
            std::shared_lock lock{ rcva_mutex_ };
            if (rcva_ != nullptr) {
                break;
            }
        }
        std::unique_lock lock{ rcva_mutex_ };
        if (rcva_ == nullptr) {
            rcva_ = std::make_shared<ColoredVertexArrayResource>(acvas);
        }
    } while (false);
    rcva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeterogeneousResource::get_animated_arrays(
    float scale) const
{
    do {
        {
            std::shared_lock lock{ acvas_mutex_ };
            if (acvas_ != nullptr) {
                break;
            }
        }
        std::unique_lock lock{ acvas_mutex_ };
        if (acvas_ == nullptr) {
            // Start with "normal" arrays.
            auto res = std::make_shared<AnimatedColoredVertexArrays>(*acvas);
            // Append hitboxes.
            bri->instantiate_hitboxes(res->dcvas, scene_node_resources_, scale);
            acvas_ = res;
        }
    } while (false);
    return acvas_;
}

void HeterogeneousResource::generate_instances() {
    static const DECLARE_REGEX(re, "^(\\w+)(?:\\.(\\d+))?\\.billboard(?:\\b|_)");
    acvas->scvas.remove_if([this](const std::shared_ptr<ColoredVertexArray<float>>& cva){
        Mlib::re::smatch match;
        if (Mlib::re::regex_search(cva->name, match, re)) {
            if (cva->triangles.empty()) {
                throw std::runtime_error("Mesh \"" + cva->name + "\" is empty");
            }
            const auto& tri = *cva->triangles.begin();
            bri->add_parsed_resource_name(
                tri(0).position.casted<double>(),
                ParsedResourceName{
                    .name = match[1].str(),
                    .billboard_id = match[2].matched ? safe_stou(match[2].str()) : UINT32_MAX,
                    .probability = NAN,
                    .aggregate_mode = scene_node_resources_.aggregate_mode(match[1].str()),
                    .create_impostor = false,
                    .hitbox = "",
                    .supplies_cooldown = NAN},
                0.f,   // yangle
                1.f);  // scale
            return true;
        } else {
            return false;
        }
    });
    for (const auto& cva : acvas->dcvas) {
        if (Mlib::re::regex_search(cva->name, re)) {
            throw std::runtime_error("Instances require single precision meshes");
        }
    }
}
