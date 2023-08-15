#include "Aggregate_Array_Renderer.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Batch_Renderers/Optional_Material_Hider.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <map>

using namespace Mlib;

static const std::string AAR_NAME = "AggregateArrayRenderer";

struct AggregateTriangle {
    FixedArray<ColoredVertex<float>, 3> triangle;
    FixedArray<uint8_t, 3> layer;
    float distance_to_origin2;
    inline operator const FixedArray<ColoredVertex<float>, 3>&() const {
        return triangle;
    }
    inline operator const FixedArray<uint8_t, 3>&() const {
        return layer;
    }
};

struct AggregateTriangles {
    std::list<AggregateTriangle> atriangles;
    bool has_texture_layers;
};

AggregateArrayRenderer::AggregateArrayRenderer()
: is_initialized_{false}
{}

AggregateArrayRenderer::~AggregateArrayRenderer() = default;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

void AggregateArrayRenderer::update_aggregates(
    const FixedArray<double, 3>& offset,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue)
{
    // size_t ntris = 0;
    // for (const auto& a : aggregate_queue) {
    //     ntris += a->triangles.size();
    // }
    // std::cerr << "Update aggregates: " << ntris << std::endl;

    //std::map<Material, size_t> material_ids;
    //size_t ntriangles = 0;
    //for (const auto& a : aggregate_queue) {
    //    if (material_ids.find(a.second.material) == material_ids.end()) {
    //        material_ids.insert(std::make_pair(a.second.material, material_ids.size()));
    //    }
    //    ntriangles += a.second.triangles->size();
    //}
    std::map<Material, AggregateTriangles> mat_lists;
    OptionalMaterialHider mhd;
    for (const auto& a : aggregate_queue) {
        if (a->triangles.empty()) {
            THROW_OR_ABORT("Aggregate triangle list is empty");
        }
        auto mat = a->material;
        if (mhd.is_hidden(mat)) {
            continue;
        }
        mat.aggregate_mode = AggregateMode::NONE;
        mat.center_distances = default_step_distances;
        auto it = mat_lists.find(mat);
        if (it == mat_lists.end()) {
            it = mat_lists.insert({mat, {}}).first;
            it->second.has_texture_layers = !a->triangle_texture_layers.empty();
        }
        auto& l = it->second;
        if (a->triangle_texture_layers.empty() == l.has_texture_layers) {
            THROW_OR_ABORT("Inconsistent aggregate triangle_texture_layers between lists");
        }
        if (l.has_texture_layers &&
            (a->triangle_texture_layers.size() != a->triangles.size()))
        {
            THROW_OR_ABORT("Layer information differs from triangle list length");
        }
        auto max_distance2 = squared(mat.max_triangle_distance);
        for (size_t i = 0; i < a->triangles.size(); ++i) {
            const auto& c = a->triangles[i];
            auto distance_to_origin2 = sum(squared(c(0).position + c(1).position + c(2).position));
            if ((max_distance2 != INFINITY) &&
                (distance_to_origin2 > max_distance2))
            {
                continue;
            }
            if (l.has_texture_layers) {
                l.atriangles.push_back({c, a->triangle_texture_layers[i], distance_to_origin2});
            } else {
                l.atriangles.push_back({c, {UINT8_MAX, UINT8_MAX, UINT8_MAX}, distance_to_origin2});
            }
        }
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mat_vectors;
    
    for (auto& [mat, list] : mat_lists) {
        if (list.atriangles.empty()) {
            continue;
        }
        if (any(mat.blend_mode & BlendMode::ANY_CONTINUOUS)) {
            list.atriangles.sort([](
                const AggregateTriangle& a,
                const AggregateTriangle& b)
                {
                    return a.distance_to_origin2 > b.distance_to_origin2;
                });
        }
        mat_vectors.push_back(std::make_shared<ColoredVertexArray<float>>(
            AAR_NAME,
            mat,
            PhysicsMaterial::ATTR_VISIBLE,
            ModifierBacklog{},
            std::vector<FixedArray<ColoredVertex<float>, 3>>(list.atriangles.begin(), list.atriangles.end()),
            std::vector<FixedArray<ColoredVertex<float>, 2>>(),
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>(),
            std::vector<FixedArray<std::vector<BoneWeight>, 2>>(),
            list.has_texture_layers
                ? std::vector<FixedArray<uint8_t, 3>>(list.atriangles.begin(), list.atriangles.end())
                : std::vector<FixedArray<uint8_t, 3>>(),
            std::vector<FixedArray<uint8_t, 2>>()));
    }
    auto rcva = std::make_shared<ColoredVertexArrayResource>(
        mat_vectors,
        std::list<std::shared_ptr<ColoredVertexArray<double>>>{});
    auto rcvai = std::make_unique<RenderableColoredVertexArray>(rcva, RenderableResourceFilter{});
    {
        std::scoped_lock lock_guard{mutex_};
        std::swap(rcvai_, rcvai);
        offset_ = offset;
        is_initialized_ = true;
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

void AggregateArrayRenderer::render_aggregates(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass,
    const std::list<const ColorStyle*>& color_styles) const
{
    std::scoped_lock lock_guard{mutex_};
    if (is_initialized_) {
        ColorStyle r_style;
        for (const auto& style : color_styles) {
            if (Mlib::re::regex_search(AAR_NAME, style->selector)) {
                r_style.insert(*style);
            }
        }
        TransformationMatrix<float, double, 3> m{fixed_identity_array<float, 3>(), offset_};
        rcvai_->render(
            dot2d(vp, m.affine()),
            m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            {external_render_pass, InternalRenderPass::AGGREGATE},
            nullptr,    // animation_state
            &r_style);  // color_style
    }
}

bool AggregateArrayRenderer::is_initialized() const {
    std::scoped_lock lock_guard{mutex_};
    return is_initialized_;
}

void AggregateArrayRenderer::invalidate() {
    std::scoped_lock lock_guard{mutex_};
    is_initialized_ = false;
}
