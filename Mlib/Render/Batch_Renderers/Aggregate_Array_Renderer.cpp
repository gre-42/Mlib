#include "Aggregate_Array_Renderer.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <map>

using namespace Mlib;

static const std::string AAR_NAME = "AggregateArrayRenderer";

AggregateArrayRenderer::AggregateArrayRenderer()
: is_initialized_{false}
{}

AggregateArrayRenderer::~AggregateArrayRenderer() = default;

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
    std::map<Material, std::list<FixedArray<ColoredVertex<float>, 3>>> mat_lists;
    for (const auto& a : aggregate_queue) {
        auto mat = a->material;
        mat.aggregate_mode = AggregateMode::NONE;
        mat.center_distances = default_step_distances;
        auto& l = mat_lists[mat];
        auto max_distance2 = squared(mat.max_triangle_distance);
        for (const auto& c : a->triangles) {
            if ((max_distance2 != INFINITY) &&
                (sum(squared(c(0).position + c(1).position + c(2).position)) > max_distance2))
            {
                continue;
            }
            l.push_back(c);
        }
        if (any(a->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
            l.sort([](
                const FixedArray<ColoredVertex<float>, 3>& a,
                const FixedArray<ColoredVertex<float>, 3>& b)
                {
                    return sum(squared(a(0).position + a(1).position + a(2).position)) >
                        sum(squared(b(0).position + b(1).position + b(2).position));
                });
        }
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mat_vectors;
    for (const auto& [mat, list] : mat_lists) {
        if (list.empty()) {
            continue;
        }
        mat_vectors.push_back(std::make_shared<ColoredVertexArray<float>>(
            AAR_NAME,
            mat,
            PhysicsMaterial::ATTR_VISIBLE,
            std::vector<FixedArray<ColoredVertex<float>, 3>>{list.begin(), list.end()},
            std::vector<FixedArray<ColoredVertex<float>, 2>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 2>>{}));
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
