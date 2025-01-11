#include "Aggregate_Array_Renderer.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Batch_Renderers/Optional_Material_Hider.hpp>
#include <Mlib/Render/Batch_Renderers/Optional_Mesh_Hider.hpp>
#include <Mlib/Render/Batch_Renderers/Special_Renderable_Names.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Yield.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <map>

using namespace Mlib;

enum class TextureLayerType {
    NONE,
    CONTINUOUS,
    DISCRETE
};

template <TextureLayerType ttexture_layer_type>
struct AggregateTriangle;

template <>
struct AggregateTriangle<TextureLayerType::NONE> {
    FixedArray<ColoredVertex<float>, 3> triangle;
    float distance_to_origin2;
};

template <>
struct AggregateTriangle<TextureLayerType::CONTINUOUS> {
    FixedArray<ColoredVertex<float>, 3> triangle;
    FixedArray<float, 3> continuous_layer;
    float distance_to_origin2;
};

template <>
struct AggregateTriangle<TextureLayerType::DISCRETE> {
    FixedArray<ColoredVertex<float>, 3> triangle;
    FixedArray<uint8_t, 3> discrete_layer;
    float distance_to_origin2;
};

class IAggregateTriangles {
public:
    virtual ~IAggregateTriangles() = default;
    virtual void append(
        const ColoredVertexArray<float>& a,
        std::minstd_rand& rng,
        const ExternalRenderPass& external_render_pass) = 0;
    virtual void build(
        UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
        UUVector<FixedArray<float, 3>>& continuous_triangle_texture_layers,
        UUVector<FixedArray<uint8_t, 3>>& discrete_triangle_texture_layers) = 0;
    virtual void sort() = 0;
    virtual bool empty() const = 0;
};

template <TextureLayerType ttexture_layer_type>
struct AggregateTriangles: public IAggregateTriangles {
public:
    virtual void append(
        const ColoredVertexArray<float>& a,
        std::minstd_rand& rng,
        const ExternalRenderPass& external_render_pass) override
    {
        if (a.triangles.empty()) {
            THROW_OR_ABORT("Detected empty triangles in array \"" + a.name + '"');
        }
        if constexpr (ttexture_layer_type == TextureLayerType::NONE) {
            if (!a.continuous_triangle_texture_layers.empty()) {
                THROW_OR_ABORT("Unexpected continuous texture layers in array \"" + a.name + '"');
            }
            if (!a.discrete_triangle_texture_layers.empty()) {
                THROW_OR_ABORT("Unexpected discrete texture layers in array \"" + a.name + '"');
            }
        }
        if constexpr (ttexture_layer_type == TextureLayerType::CONTINUOUS) {
            if (a.continuous_triangle_texture_layers.size() != a.triangles.size()) {
                THROW_OR_ABORT("Conflicting number of continuous texture layers in array \"" + a.name + '"');
            }
            if (!a.discrete_triangle_texture_layers.empty()) {
                THROW_OR_ABORT("Unexpected discrete texture layers in array \"" + a.name + '"');
            }
        }
        if constexpr (ttexture_layer_type == TextureLayerType::DISCRETE) {
            if (!a.continuous_triangle_texture_layers.empty()) {
                THROW_OR_ABORT("Unexpected continuous texture layers in array \"" + a.name + '"');
            }
            if (a.discrete_triangle_texture_layers.size() != a.triangles.size()) {
                THROW_OR_ABORT("Conflicting number of texture layers in array \"" + a.name + '"');
            }
        }
        auto camera_sphere = BoundingSphere<float, 3>{ fixed_zeros<float, 3>(), a.morphology.max_triangle_distance };
        for (size_t i = 0; i < a.triangles.size(); ++i) {
            if (i % THREAD_YIELD_INTERVAL == 0) {
                std::this_thread::yield();
            }
            const auto& c = a.triangles[i];
            // auto triangle_sphere = welzl_from_fixed(FixedArray<FixedArray<float, 3>, 3>{ c(0).position, c(1).position, c(2).position }, rng);
            auto triangle_sphere = BoundingSphere<float, 3>{ FixedArray<float, 3, 3>{ c(0).position, c(1).position, c(2).position } };
            if (!any(external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK) &&
                (a.morphology.max_triangle_distance != INFINITY) &&
                !camera_sphere.intersects(triangle_sphere))
            {
                continue;
            }
            auto distance_to_origin2 = sum(squared(triangle_sphere.center));
            if constexpr (ttexture_layer_type == TextureLayerType::NONE) {
                atriangles_.push_back({ c, distance_to_origin2 });
            }
            if constexpr (ttexture_layer_type == TextureLayerType::CONTINUOUS) {
                atriangles_.push_back({ c, a.continuous_triangle_texture_layers[i], distance_to_origin2 });
            }
            if constexpr (ttexture_layer_type == TextureLayerType::DISCRETE) {
                atriangles_.push_back({ c, a.discrete_triangle_texture_layers[i], distance_to_origin2 });
            }
        }
    }
    virtual void build(
        UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
        UUVector<FixedArray<float, 3>>& continuous_triangle_texture_layers,
        UUVector<FixedArray<uint8_t, 3>>& discrete_triangle_texture_layers) override
    {
        assert_true(triangles.empty());
        assert_true(continuous_triangle_texture_layers.empty());
        assert_true(discrete_triangle_texture_layers.empty());
        triangles.reserve(atriangles_.size());
        if constexpr (ttexture_layer_type == TextureLayerType::CONTINUOUS) {
            continuous_triangle_texture_layers.reserve(atriangles_.size());
        }
        if constexpr (ttexture_layer_type == TextureLayerType::DISCRETE) {
            discrete_triangle_texture_layers.reserve(atriangles_.size());
        }
        for (const auto& a : atriangles_) {
            triangles.push_back(a.triangle);
            if constexpr (ttexture_layer_type == TextureLayerType::CONTINUOUS) {
                continuous_triangle_texture_layers.push_back(a.continuous_layer);
            }
            if constexpr (ttexture_layer_type == TextureLayerType::DISCRETE) {
                discrete_triangle_texture_layers.push_back(a.discrete_layer);
            }
        }
    }
    virtual void sort() override {
        atriangles_.sort([](
            const AggregateTriangle<ttexture_layer_type>& a,
            const AggregateTriangle<ttexture_layer_type>& b)
            {
                return a.distance_to_origin2 > b.distance_to_origin2;
            });
    }
    virtual bool empty() const override {
        return atriangles_.empty();
    }
private:
    std::list<AggregateTriangle<ttexture_layer_type>> atriangles_;
};

static std::unique_ptr<IAggregateTriangles> construct_aggregate_triangles(
    const ColoredVertexArray<float>& a,
    std::minstd_rand& rng,
    const ExternalRenderPass& external_render_pass)
{
    std::unique_ptr<IAggregateTriangles> result;
    if (!a.continuous_triangle_texture_layers.empty() &&
        !a.discrete_triangle_texture_layers.empty())
    {
        THROW_OR_ABORT("Detected continuous and discrete texture layers");
    }
    if (!a.continuous_triangle_texture_layers.empty()) {
        result = std::make_unique<AggregateTriangles<TextureLayerType::CONTINUOUS>>();
    } else if (!a.discrete_triangle_texture_layers.empty()) {
        result = std::make_unique<AggregateTriangles<TextureLayerType::DISCRETE>>();
    } else {
        result = std::make_unique<AggregateTriangles<TextureLayerType::NONE>>();
    }
    result->append(a, rng, external_render_pass);
    return result;
}

AggregateArrayRenderer::AggregateArrayRenderer(RenderingResources& rendering_resources)
    : rendering_resources_{ rendering_resources }
    , offset_((ScenePos)NAN)
    , next_offset_{ uninitialized }
    , is_initialized_{ false }
{}

AggregateArrayRenderer::~AggregateArrayRenderer() = default;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

void AggregateArrayRenderer::update_aggregates(
    const FixedArray<ScenePos, 3>& offset,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
    const ExternalRenderPass& external_render_pass,
    TaskLocation task_location)
{
    {
        std::scoped_lock lock_guard{ mutex_ };
        if (next_rcva_ != nullptr) {
            // lwarn() << "Aggregated arrays were not drawn at all (transfer to GPU too slow, can happen for small scenes)";
            return;
        }
    }
    // size_t ntris = 0;
    // for (const auto& a : aggregate_queue) {
    //     ntris += a->triangles.size();
    // }
    // lerr() << "Update aggregates: " << ntris;

    //std::map<Material, size_t> material_ids;
    //size_t ntriangles = 0;
    //for (const auto& a : aggregate_queue) {
    //    if (material_ids.find(a.second.material) == material_ids.end()) {
    //        material_ids.insert(std::make_pair(a.second.material, material_ids.size()));
    //    }
    //    ntriangles += a.second.triangles->size();
    //}
    std::map<Material, std::unique_ptr<IAggregateTriangles>> mat_lists;
    OptionalMaterialHider mhd;
    OptionalMeshHider nhd;
    auto rng = welzl_rng();
    for (const auto& a : aggregate_queue) {
        if (a->triangles.empty()) {
            THROW_OR_ABORT("Aggregate triangle list is empty: \"" + a->name + '"');
        }
        if (nhd.is_hidden(a->name)) {
            continue;
        }
        auto mat = a->material;
        if (mhd.is_hidden(mat)) {
            continue;
        }
        mat.aggregate_mode = AggregateMode::NONE;
        auto it = mat_lists.find(mat);
        if (it == mat_lists.end()) {
            auto l = construct_aggregate_triangles(*a, rng, external_render_pass);
            if (!l->empty() && !mat_lists.try_emplace(mat, std::move(l)).second) {
                verbose_abort("Internal error in AggregateArrayRenderer::update_aggregates");
            }
        } else {
            it->second->append(*a, rng, external_render_pass);
        }
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mat_vectors;
    
    for (auto& [mat, list] : mat_lists) {
        if (any(mat.blend_mode & BlendMode::ANY_CONTINUOUS)) {
            list->sort();
        }
        UUVector<FixedArray<ColoredVertex<float>, 3>> triangles;
        UUVector<FixedArray<float, 3>> continuous_texture_layers;
        UUVector<FixedArray<uint8_t, 3>> discrete_texture_layers;
        list->build(triangles, continuous_texture_layers, discrete_texture_layers);
        mat_vectors.push_back(std::make_shared<ColoredVertexArray<float>>(
            *AAR_NAME,
            mat,
            Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE },
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            std::move(continuous_texture_layers),
            std::move(discrete_texture_layers),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>()));
    }
    auto rcva = std::make_shared<ColoredVertexArrayResource>(
        mat_vectors,
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{});
    auto rcvai = mat_vectors.empty()
        ? nullptr
        : std::make_unique<RenderableColoredVertexArray>(rendering_resources_, rcva, RenderableResourceFilter{});
    if (task_location == TaskLocation::FOREGROUND) {
        rcva->wait();
    }
    {
        std::scoped_lock lock_guard{ mutex_ };
        if (next_rcva_ != nullptr) {
            verbose_abort("AggregateArrayRenderer::update_aggregates called in parallel");
        }
        std::swap(next_rcva_, rcva);
        std::swap(next_rcvai_, rcvai);
        next_offset_ = offset;
        is_initialized_ = true;
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

void AggregateArrayRenderer::render_aggregates(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass,
    const std::list<const ColorStyle*>& color_styles) const
{
    std::unique_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        return;
    }
    if ((next_rcva_ != nullptr) && !next_rcva_->copy_in_progress()) {
        next_rcva_ = nullptr;
        rcvai_ = std::move(next_rcvai_);
        offset_ = next_offset_;
    }
    if (rcvai_ == nullptr) {
        return;
    }
    if (any(isnan(offset_))) {
        verbose_abort("Offset is NAN");
    }
    lock_guard.unlock();
    ColorStyle r_style;
    for (const auto& style : color_styles) {
        if (style->matches(AAR_NAME)) {
            r_style.insert(*style);
        }
    }
    TransformationMatrix<float, ScenePos, 3> m{fixed_identity_array<float, 3>(), offset_};
    rcvai_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        nullptr,    // dynamic style
        lights,
        skidmarks,
        scene_graph_config,
        render_config,
        { external_render_pass, InternalRenderPass::AGGREGATE },
        nullptr,    // animation_state
        &r_style);  // color_style
}

bool AggregateArrayRenderer::is_initialized() const {
    std::scoped_lock lock_guard{ mutex_ };
    return is_initialized_;
}

void AggregateArrayRenderer::invalidate() {
    std::scoped_lock lock_guard{ mutex_ };
    is_initialized_ = false;
    next_rcva_ = nullptr;
    rcvai_ = nullptr;
    next_rcvai_ = nullptr;
}

FixedArray<ScenePos, 3> AggregateArrayRenderer::offset() const {
    std::scoped_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        THROW_OR_ABORT("AggregateArrayRenderer not initialized, cannot return offset");
    }
    return offset_;
}
