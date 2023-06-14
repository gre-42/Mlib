#include "Heterogeneous_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

HeterogeneousResource::HeterogeneousResource(
    const SceneNodeResources& scene_node_resources,
    const FixedArray<float, 3>& instance_rotation,
    float instance_scale)
: bri{ std::make_unique<BatchResourceInstantiator>(instance_rotation, instance_scale) },
  acvas{ std::make_shared<AnimatedColoredVertexArrays>() },
  scene_node_resources_{ scene_node_resources }
{}

HeterogeneousResource::~HeterogeneousResource()
{}

// ISceneNodeResource

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
    bri->instantiate_renderables(scene_node_resources_, options);

    do {
        {
            std::shared_lock lock{ rcva_mutex_ };
            if (rcva_ != nullptr) {
                break;
            }
        }
        std::scoped_lock lock{ rcva_mutex_ };
        if (rcva_ == nullptr) {
            rcva_ = std::make_shared<ColoredVertexArrayResource>(acvas);
        }
    } while (false);
    rcva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeterogeneousResource::get_animated_arrays() const {
    do {
        {
            std::shared_lock lock{ acvas_mutex_ };
            if (acvas_ != nullptr) {
                break;
            }
        }
        std::scoped_lock lock{ acvas_mutex_ };
        if (acvas_ == nullptr) {
            // Start with "normal" arrays.
            auto res = std::make_shared<AnimatedColoredVertexArrays>(*acvas);
            // Append hitboxes.
            bri->instantiate_hitboxes(res->dcvas, scene_node_resources_);
            acvas_ = res;
        }
    } while (false);
    return acvas_;
}

void HeterogeneousResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    ColoredVertexArrayResource(acvas).generate_triangle_rays(npoints, lengths, delete_triangles);
}

void HeterogeneousResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    ColoredVertexArrayResource(acvas).generate_ray(from, to);
}

std::shared_ptr<ISceneNodeResource> HeterogeneousResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return ColoredVertexArrayResource(acvas).generate_grind_lines(edge_angle, averaged_normal_angle, filter);
}

std::shared_ptr<ISceneNodeResource> HeterogeneousResource::generate_contour_edges() const {
    return ColoredVertexArrayResource(acvas).generate_contour_edges();
}

void HeterogeneousResource::convex_decompose_terrain(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter) const
{
    ColoredVertexArrayResource(acvas).convex_decompose_terrain(depth, destination_physics_material, filter);
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

void HeterogeneousResource::generate_instances() {
    static const DECLARE_REGEX(re, "^(\\w+)(?:\\.(\\d+))?\\.billboard(?:\\b|_)");
    acvas->scvas.remove_if([this](const std::shared_ptr<ColoredVertexArray<float>>& cva){
        Mlib::re::smatch match;
        if (Mlib::re::regex_search(cva->name, match, re)) {
            if (cva->triangles.empty()) {
                THROW_OR_ABORT("Mesh \"" + cva->name + "\" is empty");
            }
            const auto& tri = *cva->triangles.begin();
            bri->add_parsed_resource_name(
                tri(0).position.casted<double>(),
                ParsedResourceName{
                    .name = match[1].str(),
                    .billboard_id = match[2].matched ? safe_stou(match[2].str()) : UINT32_MAX,
                    .probability = NAN,
                    .probability1 = NAN,
                    .min_distance_to_bdry = NAN,
                    .max_distance_to_bdry = NAN,
                    .aggregate_mode = scene_node_resources_.aggregate_mode(match[1].str()),
                    .create_imposter = false,
                    .max_imposter_texture_size = 0,
                    .hitbox = "",
                    .supplies = {},
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
            THROW_OR_ABORT("Instances require single precision meshes");
        }
    }
}
