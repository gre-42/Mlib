#include "Heterogeneous_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

HeterogeneousResource::HeterogeneousResource(
    const SceneNodeResources& scene_node_resources,
    const FixedArray<float, 3>& instance_rotation,
    float instance_scale,
    const TransformationMatrix<double, double, 3>& geographic_mapping)
    : bri{ std::make_unique<BatchResourceInstantiator>(instance_rotation, instance_scale) }
    , acvas{ std::make_shared<AnimatedColoredVertexArrays>() }
    , scene_node_resources_{ scene_node_resources }
    , geographic_mapping_{geographic_mapping}
{}

HeterogeneousResource::~HeterogeneousResource()
{}

// ISceneNodeResource

void HeterogeneousResource::preload(const RenderableResourceFilter& filter) const {
    bri->preload(scene_node_resources_, filter);
    auto preload_textures = [&filter](const auto& cvas) {
        for (const auto& [i, cva] : enumerate(cvas)) {
            if (!filter.matches(i, *cva)) {
                continue;
            }
            for (const auto& tex : cva->material.textures_color) {
                RenderingContextStack::primary_rendering_resources().preload(tex.texture_descriptor);
            }
            for (const auto& tex : cva->material.textures_alpha) {
                RenderingContextStack::primary_rendering_resources().preload(tex.texture_descriptor);
            }
        }
    };
    preload_textures(acvas->scvas);
    preload_textures(acvas->dcvas);
}

void HeterogeneousResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    bri->instantiate_root_renderables(scene_node_resources_, options);

    if (!acvas->scvas.empty() || !acvas->dcvas.empty()) {
        auto node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
            options.absolute_model_matrix.get_scale(),
            PoseInterpolationMode::DISABLED);
        instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = VariableAndHash{ *options.instance_name + "_hri_arrays" },
            .scene_node = node.ref(DP_LOC),
            .renderable_resource_filter = options.renderable_resource_filter});
        options.scene.auto_add_root_node(*options.instance_name + "_hri_world", std::move(node), RenderingDynamics::STATIC);
    }
}

void HeterogeneousResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const {
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
    rcva_->instantiate_child_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeterogeneousResource::get_physics_arrays() const {
    do {
        {
            std::shared_lock lock{ physics_arrays_mutex_ };
            if (physics_arrays_ != nullptr) {
                break;
            }
        }
        std::scoped_lock lock{ physics_arrays_mutex_ };
        if (physics_arrays_ == nullptr) {
            // Start with "normal" arrays.
            auto res = std::make_shared<AnimatedColoredVertexArrays>(*acvas);
            // Append hitboxes.
            bri->instantiate_hitboxes(res->dcvas, scene_node_resources_);
            physics_arrays_ = res;
        }
    } while (false);
    return physics_arrays_;
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> HeterogeneousResource::get_rendering_arrays() const {
    return { acvas };
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> HeterogeneousResource::get_intersectables() const {
    return {};
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

void HeterogeneousResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    ColoredVertexArrayResource(acvas).create_barrier_triangle_hitboxes(depth, destination_physics_material, filter);
}

void HeterogeneousResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    return ColoredVertexArrayResource(acvas).modify_physics_material_tags(add, remove, filter);
}

AggregateMode HeterogeneousResource::get_aggregate_mode() const {
    return ColoredVertexArrayResource(acvas).get_aggregate_mode();
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
        if (Mlib::re::regex_search(cva->name.full_name(), match, re)) {
            if (cva->triangles.empty()) {
                THROW_OR_ABORT("Mesh \"" + cva->name.full_name() + "\" is empty");
            }
            const auto& tri = *cva->triangles.begin();
            bri->add_parsed_resource_name(
                tri(0).position.casted<CompressedScenePos>(),
                ParsedResourceName{
                    .name = VariableAndHash{ match[1].str() },
                    .billboard_id = match[2].matched ? safe_stox<BillboardId>(match[2].str()) : BILLBOARD_ID_NONE,
                    .yangle = 0.f,
                    .probability = NAN,
                    .probability1 = NAN,
                    .min_distance_to_bdry = NAN,
                    .max_distance_to_bdry = NAN,
                    .aggregate_mode = scene_node_resources_.aggregate_mode(match[1].str()),
                    .create_imposter = false,
                    .max_imposter_texture_size = 0,
                    .hitbox = VariableAndHash<std::string>{},
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
        if (Mlib::re::regex_search(cva->name.full_name(), re)) {
            THROW_OR_ABORT("Instances require single precision meshes");
        }
    }
}

void HeterogeneousResource::smoothen_edges(
    SmoothnessTarget target,
    float smoothness,
    size_t niterations,
    float decay)
{
    acvas->smoothen_edges(target, smoothness, niterations, decay);
}

TransformationMatrix<double, double, 3> HeterogeneousResource::get_geographic_mapping(
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    return geographic_mapping_;
}
