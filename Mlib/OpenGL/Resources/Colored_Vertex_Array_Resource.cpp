#include "Colored_Vertex_Array_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Interior_Texture_Set.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Material_Features.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Import_Bone_Weights.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Cluster_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Cluster_Triangles.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Position_And_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Rays.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Map/Ordered_Map.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/Context_Query.hpp>
#include <Mlib/OpenGL/Deallocate/Render_Deallocator.hpp>
#include <Mlib/OpenGL/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/OpenGL/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Distant_Triangle_Hider.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instances/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/IInstantiation_Reference.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>
#include <Mlib/Scene_Graph/Render/Caching_Behavior.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <iostream>
#include <mutex>

using namespace Mlib;

ColoredVertexArrayResource::ColoredVertexArrayResource(
    std::shared_ptr<AnimatedColoredVertexArrays> triangles,
    std::list<std::shared_ptr<IGpuVertexData>> gpu_vertex_data,
    std::list<std::shared_ptr<IGpuVertexArray>> gpu_vertex_arrays)
    : ISceneNodeResource{"ColoredVertexArrayResource"}
    , scene_node_resources_{ RenderingContextStack::primary_scene_node_resources() }
    , rendering_resources_{ RenderingContextStack::primary_rendering_resources() }
    , gpu_object_factory_{ RenderingContextStack::primary_gpu_object_factory() }
    , triangles_res_{ std::move(triangles) }
    , gpu_vertex_data_{ std::move(gpu_vertex_data) }
    , gpu_vertex_arrays_{ std::move(gpu_vertex_arrays) }
{
#ifdef DEBUG
    if (triangles_res_ != nullptr) {
        triangles_res_->check_consistency();
    }
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    std::list<std::shared_ptr<IGpuVertexArray>> gpu_vertex_arrays)
    : ColoredVertexArrayResource{nullptr, {}, gpu_vertex_arrays}
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    std::shared_ptr<IGpuVertexArray> gpu_vertex_array)
    : ColoredVertexArrayResource{nullptr, {}, {gpu_vertex_array}}
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    std::shared_ptr<IGpuVertexData> gpu_vertex_data,
    std::shared_ptr<IGpuInstanceBuffers> gpu_instances)
    : ColoredVertexArrayResource{nullptr, {}, {}}
{
    gpu_vertex_arrays_.push_back(gpu_object_factory_.create_vertex_array(
        gpu_vertex_data, gpu_instances));
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles)
: ColoredVertexArrayResource{
    std::make_shared<AnimatedColoredVertexArrays>(), {}, {}}
{
    triangles_res_->scvas = striangles;
    triangles_res_->dcvas = dtriangles;
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles)
    : ColoredVertexArrayResource{
        std::list<std::shared_ptr<ColoredVertexArray<float>>>{striangles},
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{}}
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& dtriangles)
    : ColoredVertexArrayResource{
        std::list<std::shared_ptr<ColoredVertexArray<float>>>{},
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{dtriangles}}
{}

// From: https://stackoverflow.com/questions/26379311/calling-initializer-list-constructor-via-make-unique-make-shared
template<typename T>
static std::initializer_list<T> make_init_list(std::initializer_list<T>&& l) {
    return l;
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<AnimatedColoredVertexArrays>& triangles)
    : ColoredVertexArrayResource{ triangles, {}, {} }
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles)
    : ColoredVertexArrayResource{ striangles, std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{} }
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles)
    : ColoredVertexArrayResource{ std::list<std::shared_ptr<ColoredVertexArray<float>>>{}, dtriangles }
{}

ColoredVertexArrayResource::~ColoredVertexArrayResource() = default;

void ColoredVertexArrayResource::preload(const RenderableResourceFilter& filter) const {
    if (triangles_res_ != nullptr) {
        auto preload_textures = [this, &filter](const auto& cvas) {
            for (const auto& [i, cva] : enumerate(cvas)) {
                if (!filter.matches(i, *cva)) {
                    continue;
                }
                for (auto& t : cva->meta.material.textures_color) {
                    rendering_resources_.preload(t.texture_descriptor);
                }
                for (auto& t : cva->meta.material.textures_alpha) {
                    rendering_resources_.preload(t.texture_descriptor);
                }
            }
        };
        preload_textures(triangles_res_->scvas);
        preload_textures(triangles_res_->dcvas);
    }
    if (ContextQuery::is_initialized()) {
        if (triangles_res_ != nullptr) {
            for (const auto &cva : triangles_res_->scvas) {
                if (requires_aggregation(*cva)) {
                    continue;
                }
                scene_node_resources_.get_gpu_vertex_data(cva, triangles_res_)->wait();
            }
        }
        for (const auto& a : gpu_vertex_arrays_) {
            a->preload();
        }
    }
}

void ColoredVertexArrayResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
#ifdef DEBUG
    if (triangles_res_ != nullptr) {
        triangles_res_->check_consistency();
    }
#endif
    if (options.rendering_resources == nullptr) {
        throw std::runtime_error("ColoredVertexArrayResource::instantiate_child_renderable: rendering-resources is null");
    }
    options.scene_node->add_renderable(options.instance_name, std::make_shared<RenderableColoredVertexArray>(
        *options.rendering_resources,
        shared_from_this(),
        CachingBehavior::ENABLED,
        options.renderable_resource_filter));
}

void ColoredVertexArrayResource::instantiate_root_renderables(const RootInstantiationOptions& options) const {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("instantiate_root_renderables requires triangles_res");
    }
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dcvas_node_triangles;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dcvas_node_object;
    auto dcvas_default = triangles_res_->dcvas;
    dcvas_default.remove_if([&](auto& cva){
        switch (cva->meta.material.aggregate_mode) {
        case AggregateMode::NODE_TRIANGLES:
            dcvas_node_triangles.push_back(cva);
            return true;
        case AggregateMode::NODE_OBJECT:
            dcvas_node_object.push_back(cva);
            return true;
        default:
            return false;
        }
    });
    auto instantiate_position_and_meshes = [&](const std::string& suffix, const PositionAndMeshes<CompressedScenePos>& cs){
        auto tm = TranslationMatrix{ cs.position.casted<ScenePos>() };
        auto trafo = options.absolute_model_matrix * tm;
        auto node = make_unique_scene_node(
            trafo.t,
            matrix_2_tait_bryan_angles(trafo.R),
            trafo.get_scale(),
            std::nullopt,
            PoseInterpolationMode::DISABLED);
        for (const auto& [continuous_blending_z_order, cvas] : cs.cvas) {
            std::list<std::shared_ptr<ColoredVertexArray<float>>> scvas;
            {
                size_t nelements = 0;
                for (const auto& cva : cvas) {
                    auto scva = cva->translated<float>(-cs.position, "_centered");
                    scva->meta.morphology.center_distances2 += scva->radius(fixed_zeros<float, 3>());
                    scvas.emplace_back(std::move(scva));
                    nelements += cva->nelements();
                }
                if (nelements > 1'000'000) {
                    for (const auto& cva : cvas) {
                        cva->print_stats(lerr().ref());
                    }
                    throw std::runtime_error("Clustered mesh has too many elements. Suffix: \"" + suffix + "\", #elements: " + std::to_string(nelements));
                }
            }
            auto rcva = std::make_shared<ColoredVertexArrayResource>(std::move(scvas));
            rcva->instantiate_child_renderable(ChildInstantiationOptions{
                .rendering_resources = options.rendering_resources,
                .instance_name = VariableAndHash<std::string>{
                    *options.instance_name + suffix + '_' + std::to_string(continuous_blending_z_order) },
                .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
                .renderable_resource_filter = options.renderable_resource_filter});
        }
        if (options.max_imposter_texture_size != 0) {
            if (options.imposters == nullptr) {
                throw std::runtime_error("Imposters not set for \"" + *options.instance_name + '"');
            }
            auto imposter_node_name = *options.instance_name + "-" + std::to_string(options.scene.get_uuid());
            options.imposters->set_imposter_info(node.ref(CURRENT_SOURCE_LOCATION), { imposter_node_name, options.max_imposter_texture_size });
        }
        auto node_name = VariableAndHash<std::string>{*options.instance_name + suffix};
        options.scene.auto_add_root_node(
            node_name,
            std::move(node),
            RenderingDynamics::STATIC);
        if (options.instantiated_nodes != nullptr) {
            options.instantiated_nodes->emplace_back(std::move(node_name));
        }
    };
    for (const auto& [triangle_cluster_width, dcvas] : triangle_cluster_width_groups(dcvas_node_triangles)) {
        if (triangle_cluster_width == 0) {
            std::stringstream sstr;
            for (const auto& cva : dcvas) {
                sstr << '"' << cva->meta.name << "\"\n";
            }
            throw std::runtime_error("Aggregate-mode is \"node_triangles\", \"triangle_cluster_width\" is zero:\n" + sstr.str());
        }
        for (const auto& [i, cs] : enumerate(cluster_triangles(
            dcvas,
            cluster_center_by_grid<CompressedScenePos, ScenePos>(fixed_full<ScenePos, 3>(triangle_cluster_width)),
            *options.instance_name + "_split")))
        {
            instantiate_position_and_meshes(
                "_triangle_cluster" + std::to_string(triangle_cluster_width) + '_' + std::to_string(i),
                cs);
        }
    }
    for (const auto& [object_cluster_width, dcvas] : object_cluster_width_groups(dcvas_node_object)) {
        if (object_cluster_width == 0) {
            std::stringstream sstr;
            for (const auto& cva : dcvas) {
                sstr << '"' << cva->meta.name << "\"\n";
            }
            throw std::runtime_error("Aggregate-mode is \"node_object\", \"object_cluster_width\" is zero:\n" + sstr.str());
        }
        for (const auto& [i, cs] : enumerate(cluster_meshes<CompressedScenePos>(
            dcvas,
            cva_to_grid_center<CompressedScenePos, ScenePos>(fixed_full<ScenePos, 3>(object_cluster_width)),
            *options.instance_name + "_mesh_cluster")))
        {
            instantiate_position_and_meshes(
                "_mesh_cluster" + std::to_string(object_cluster_width) + '_' + std::to_string(i),
                cs);
        }
    }
    if (!triangles_res_->scvas.empty() || !dcvas_default.empty()) {
        auto node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
            options.absolute_model_matrix.get_scale(),
            std::nullopt,
            PoseInterpolationMode::DISABLED);
        instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = options.instance_name,
            .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
            .renderable_resource_filter = options.renderable_resource_filter});
        if (options.max_imposter_texture_size != 0) {
            if (options.imposters == nullptr) {
                throw std::runtime_error("Imposters not set for \"" + *options.instance_name + '"');
            }
            auto imposter_node_name = *options.instance_name + "-" + std::to_string(options.scene.get_uuid());
            options.imposters->set_imposter_info(node.ref(CURRENT_SOURCE_LOCATION), { imposter_node_name, options.max_imposter_texture_size });
        }
        auto node_name = VariableAndHash<std::string>{*options.instance_name + "_cva_world"};
        options.scene.auto_add_root_node(
            node_name,
            std::move(node),
            RenderingDynamics::STATIC);
        if (options.instantiated_nodes != nullptr) {
            options.instantiated_nodes->emplace_back(std::move(node_name));
        }
    }
}

std::shared_ptr<AnimatedColoredVertexArrays> ColoredVertexArrayResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    if (triangles_res_ != nullptr) {
        result->skeleton = triangles_res_->skeleton;
        result->bone_indices = triangles_res_->bone_indices;
        for (const auto& cva : triangles_res_->scvas) {
            if (filter.matches(*cva)) {
                result->scvas.push_back(cva);
            }
        }
        for (const auto& cva : triangles_res_->dcvas) {
            if (filter.matches(*cva)) {
                result->dcvas.push_back(cva);
            }
        }
    }
    return result;
}

void ColoredVertexArrayResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("generate_triangle_rays requires exactly triangles_res");
    }
    auto gen_triangle_rays = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        for (auto& t : cvas) {
            auto r = Mlib::generate_triangle_rays(t->triangles, npoints, lengths.template casted<TPos>());
            t->lines.reserve(t->lines.size() + r.size());
            for (const auto& l : r) {
                t->lines.emplace_back(
                    ColoredVertex<TPos>{
                        l[0],
                        Colors::WHITE,
                        {0.f, 0.f}
                    },
                    ColoredVertex<TPos>{
                        l[1],
                        Colors::WHITE,
                        {0.f, 1.f}
                    });
            }
            if (delete_triangles) {
                t->triangles.clear();
            }
        }
    };
    gen_triangle_rays(triangles_res_->scvas);
    gen_triangle_rays(triangles_res_->dcvas);
}

void ColoredVertexArrayResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    if ((triangles_res_ == nullptr) || (triangles_res_->scvas.size() != 1)) {
        throw std::runtime_error("generate_ray requires exactly one single precision triangle mesh");
    }
    triangles_res_->scvas.front()->lines.emplace_back(
        ColoredVertex<float>{
            from,
            Colors::WHITE,
            {0.f, 0.f},
            fixed_zeros<float, 3>(),
            fixed_zeros<float, 3>(),
        },
        ColoredVertex<float>{
            to,
            Colors::WHITE,
            {0.f, 1.f},
            fixed_zeros<float, 3>(),
            fixed_zeros<float, 3>(),
        });
}

std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("generate_grind_lines requires triangles_res");
    }
    return std::make_shared<ColoredVertexArrayResource>(
        triangles_res_->generate_grind_lines(edge_angle, averaged_normal_angle, filter));
}

std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::generate_contour_edges() const {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("generate_contour_edges requires triangles_res");
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> dest_scvas;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dest_dcvas;
    for (auto& t : triangles_res_->scvas) {
        dest_scvas.push_back(
            t->generate_contour_edges());
    }
    for (auto& t : triangles_res_->dcvas) {
        dest_dcvas.push_back(
            t->generate_contour_edges());
    }
    return std::make_shared<ColoredVertexArrayResource>(dest_scvas, dest_dcvas);
}

void ColoredVertexArrayResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("create_barrier_triangle_hitboxes requires triangles_res");
    }
    triangles_res_->create_barrier_triangle_hitboxes(depth, destination_physics_material, filter);
}

// std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::extract_by_predicate(
//     const std::function<bool(const ColoredVertexArray& cva)>& predicate)
// {
//     std::list<std::shared_ptr<ColoredVertexArray>> dest_cvas;
//     for (auto it = triangles_res_->cvas.begin(); it != triangles_res_->cvas.end(); )
//     {
//         if (predicate(**it)) {
//             dest_cvas.splice(dest_cvas.end(), triangles_res_->cvas, it++);
//         } else {
//             ++it;
//         }
//     }
//     return std::make_shared<ColoredVertexArrayResource>(dest_cvas);
// }

// std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::copy_by_predicate(
//     const std::function<bool(const ColoredVertexArray& cva)>& predicate)
// {
//     std::list<std::shared_ptr<ColoredVertexArray>> dest_cvas;
//     for (const auto& cva : triangles_res_->cvas) {
//         if (predicate(*cva)) {
//             dest_cvas.push_back(cva);
//         }
//     }
//     return std::make_shared<ColoredVertexArrayResource>(dest_cvas);
// }

void ColoredVertexArrayResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("modify_physics_material_tags requires triangles_res");
    }
    if (any(add & remove)) {
        throw std::runtime_error("Duplicate add/remove flags");
    }
    auto modify_tags = [&](auto& cvas){
        for (auto& cva : cvas) {
            if (filter.matches(*cva)) {
                cva->meta.morphology.physics_material |= add;
                cva->meta.morphology.physics_material &= ~remove;
                if (any(add & PhysicsMaterial::ATTR_CONTAINS_SKIDMARKS)) {
                    cva->meta.material.skidmarks |= ParticleType::SKIDMARK;
                }
            }
        }
    };
    modify_tags(triangles_res_->scvas);
    modify_tags(triangles_res_->dcvas);
}

void ColoredVertexArrayResource::downsample(size_t factor) {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("downsample requires triangles_res");
    }
    for (auto& t : triangles_res_->scvas) {
        t->downsample_triangles(factor);
    }
    for (auto& t : triangles_res_->dcvas) {
        t->downsample_triangles(factor);
    }
}

AggregateMode ColoredVertexArrayResource::get_aggregate_mode() const {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("get_aggregate_mode requires triangles_res");
    }
    std::set<AggregateMode> aggregate_modes;
    for (const auto& t : triangles_res_->scvas) {
        aggregate_modes.insert(t->meta.material.aggregate_mode);
    }
    for (const auto& t : triangles_res_->dcvas) {
        aggregate_modes.insert(t->meta.material.aggregate_mode);
    }
    if (aggregate_modes.empty()) {
        throw std::runtime_error("Cannot determine aggregate mode of empty array");
    }
    if (aggregate_modes.size() != 1) {
        throw std::runtime_error("aggregate_mode is not unique");
    }
    return *aggregate_modes.begin();
}

PhysicsMaterial ColoredVertexArrayResource::get_physics_material() const {
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("get_physics_material requires triangles_res");
    }
    auto result = PhysicsMaterial::NONE;
    for (const auto& cva : triangles_res_->scvas) {
        result |= cva->meta.morphology.physics_material;
    }
    for (const auto& cva : triangles_res_->dcvas) {
        result |= cva->meta.morphology.physics_material;
    }
    return result;
}

void ColoredVertexArrayResource::print(std::ostream& ostr) const {
    ostr << "ColoredVertexArrayResource\n";
    if (triangles_res_ == nullptr) {
        ostr << "nullptr";
    }
    triangles_res_->print_stats(ostr);
}

bool ColoredVertexArrayResource::requires_aggregation(const ColoredVertexArray<float> &cva) const {
    return any(cva.meta.material.aggregate_mode & AggregateMode::OBJECT_MASK);
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> ColoredVertexArrayResource::get_rendering_arrays() const
{
    if (triangles_res_ == nullptr) {
        return {};
    } else {
        return { triangles_res_ };
    }
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> ColoredVertexArrayResource::get_intersectables() const
{
    return {};
}

void ColoredVertexArrayResource::set_absolute_joint_poses(
    const UUVector<OffsetAndQuaternion<float, float>>& poses)
{
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("set_absolute_joint_poses requires triangles_res");
    }
    for (auto& t : triangles_res_->scvas) {
        t = t->transformed<float>(poses, "_transformed_oq");
    }
    if (!triangles_res_->dcvas.empty()) {
        throw std::runtime_error("Poses only supported for single precision");
    }
}

void ColoredVertexArrayResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    if (triangles_res_ == nullptr) {
        throw std::runtime_error("import_bone_weights requires triangles_res");
    }
    Mlib::import_bone_weights(
        *triangles_res_,
        other_acvas,
        max_distance);
}

bool ColoredVertexArrayResource::copy_in_progress() const {
    for (const auto& a : gpu_vertex_arrays_) {
        if (a->copy_in_progress()) {
            return true;
        }
    }
    return false;
}

void ColoredVertexArrayResource::wait() const {
    for (const auto& a : gpu_vertex_arrays_) {
        a->wait();
    }
}
